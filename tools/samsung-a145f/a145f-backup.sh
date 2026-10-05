#!/usr/bin/env bash
#
# a145f-backup.sh - back up a Samsung Galaxy A14 (SM-A145F) over USB
# from an Ubuntu host, before trying a new bootloader/loader.
#
# Partitions are streamed straight from the phone to this PC (nothing is
# written to the phone). A live progress bar shows per-partition and overall
# progress; everything is also logged with timestamps to <out>/backup.log.
#
# Needs: adb, and on the phone either
#   - root (Magisk/KernelSU etc. - "su" works), or
#   - a custom recovery (TWRP/OrangeFox) booted with adb running as root.
#
# Usage: ./a145f-backup.sh [options]
#   -o DIR            output directory (default: ./a145f-backup-<date>)
#   --skip-super      don't dump the (large) super partition
#   --with-userdata   also dump raw userdata (huge, usually encrypted)
#   --no-sdcard       don't copy /sdcard (internal storage) with adb pull
#   --skip a,b,c      skip these partitions by name (e.g. --skip prism,cache)
#   --with-disk       also dump the whole-disk entry mmcblk0 (huge, redundant)
#   --checksums       also write SHA256SUMS (extra pass over each file)
#   --verify          like --checksums, and re-hash on the phone and compare
#   -s SERIAL         adb serial, if several devices are attached
#   -h                help

set -uo pipefail

OUT=""
SKIP_SUPER=0
WITH_USERDATA=0
DO_SDCARD=1
CHECKSUMS=0
WITH_DISK=0
SKIP_LIST=","
VERIFY=0
SERIAL=""

usage() { awk 'NR>1 && /^#/ {sub(/^# ?/,""); print; next} NR>1 {exit}' "$0"; exit "${1:-0}"; }

while [ $# -gt 0 ]; do
	case "$1" in
		-o) OUT="${2:?missing dir}"; shift 2 ;;
		--skip-super) SKIP_SUPER=1; shift ;;
		--with-userdata) WITH_USERDATA=1; shift ;;
		--no-sdcard) DO_SDCARD=0; shift ;;
		--skip) SKIP_LIST=",${2:?missing list},"; shift 2 ;;
		--with-disk) WITH_DISK=1; shift ;;
		--checksums) CHECKSUMS=1; shift ;;
		--verify) VERIFY=1; CHECKSUMS=1; shift ;;
		-s) SERIAL="${2:?missing serial}"; shift 2 ;;
		-h|--help) usage 0 ;;
		*) echo "Unknown option: $1" >&2; usage 1 ;;
	esac
done

# ---- output + logging ------------------------------------------------------
# fd 3 is the real terminal. The progress bar only goes there; log lines go
# there AND (plain text, timestamped) into the log file.
exec 3>&1
LOGF=""
IS_TTY=0
[ -t 3 ] && IS_TTY=1

_emit() { # level color message
	local ts; ts="$(date +%H:%M:%S)"
	[ "$IS_TTY" -eq 1 ] && printf '\r\033[K' >&3
	printf '%b%s\033[0m %s\n' "$2" "$1" "$3" >&3
	[ -n "$LOGF" ] && printf '%s %s %s\n' "$ts" "$1" "$3" >> "$LOGF"
	return 0
}
log()  { _emit "[*]" "\033[1;34m" "$*"; }
warn() { _emit "[!]" "\033[1;33m" "$*"; }
die()  { _emit "[x]" "\033[1;31m" "$*"; exit 1; }

human() { numfmt --to=iec --suffix=B "$1" 2>/dev/null || echo "$1 B"; }
mmss()  { local s=$1; printf '%d:%02d' $((s / 60)) $((s % 60)); }

CUR_PID=""
cleanup() {
	[ -n "$CUR_PID" ] && kill "$CUR_PID" 2>/dev/null
	[ "$IS_TTY" -eq 1 ] && printf '\n' >&3
}
on_int() { cleanup; warn "Interrupted. Partial files remain in ${OUT:-the output dir}."; exit 130; }
trap on_int INT TERM

# ---- host checks -----------------------------------------------------------
if ! command -v adb >/dev/null 2>&1; then
	die "adb not found. Install it with:  sudo apt install adb"
fi
command -v sha256sum >/dev/null 2>&1 || die "sha256sum not found (coreutils)."

ADB=(adb)
[ -n "$SERIAL" ] && ADB=(adb -s "$SERIAL")

log "Starting adb server and looking for the phone..."
"${ADB[@]}" start-server >/dev/null 2>&1
STATE="$("${ADB[@]}" get-state 2>&1 | tr -d '\r')"
case "$STATE" in
	device|recovery|sideload) ;;
	*)
		die "No usable device (adb state: $STATE). Check the cable, enable USB debugging,
    accept the RSA prompt on the phone, and if several devices are attached use -s SERIAL.
    If adb reports 'no permissions', run:  sudo adb kill-server && sudo adb start-server
    or install udev rules:  sudo apt install android-sdk-platform-tools-common"
		;;
esac

# ---- root detection --------------------------------------------------------
ROOT_MODE=""
if [ "$("${ADB[@]}" shell id -u 2>/dev/null | tr -d '\r')" = "0" ]; then
	ROOT_MODE="direct"
else
	# try adbd root first (recovery / userdebug builds)
	if "${ADB[@]}" root >/dev/null 2>&1; then
		sleep 2
		"${ADB[@]}" wait-for-device
	fi
	if [ "$("${ADB[@]}" shell id -u 2>/dev/null | tr -d '\r')" = "0" ]; then
		ROOT_MODE="direct"
	elif "${ADB[@]}" shell 'su -c id' 2>/dev/null | tr -d '\r' | grep -q 'uid=0'; then
		ROOT_MODE="su"
	fi
fi
[ -n "$ROOT_MODE" ] || die "No root access. Root the phone (Magisk) and grant the shell
    superuser access, or reboot into TWRP/OrangeFox and run this again.
    Note: Samsung Download mode / Odin cannot read partitions back."
log "Root access: $ROOT_MODE"

# Run a command as root on the phone; binary-safe stdout. Commands must not
# contain double quotes or shell variables.
rootcmd() {
	if [ "$ROOT_MODE" = "direct" ]; then
		"${ADB[@]}" exec-out "$1"
	else
		"${ADB[@]}" exec-out "su -c \"$1\""
	fi
}

# ---- output dir ------------------------------------------------------------
[ -n "$OUT" ] || OUT="a145f-backup-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$OUT/partitions" "$OUT/info" || die "Cannot create $OUT"
LOGF="$OUT/backup.log"
: > "$LOGF"
log "Output: $(readlink -f "$OUT")"
log "Log file: $LOGF"

# ---- device sanity check ---------------------------------------------------
MODEL="$("${ADB[@]}" shell getprop ro.product.model | tr -d '\r')"
log "Device model: ${MODEL:-unknown}"
case "$MODEL" in
	SM-A145*) ;;
	*)
		warn "This doesn't look like an SM-A145F. Partition names may differ."
		read -r -p "Continue anyway? [y/N] " a
		[ "$a" = "y" ] || [ "$a" = "Y" ] || exit 1
		;;
esac

# ---- info dump -------------------------------------------------------------
log "Saving device info..."
"${ADB[@]}" shell getprop 2>/dev/null | tr -d '\r' > "$OUT/info/getprop.txt"
rootcmd 'cat /proc/partitions' 2>/dev/null | tr -d '\r' > "$OUT/info/proc_partitions.txt"
rootcmd 'cat /proc/cmdline' 2>/dev/null | tr -d '\r' > "$OUT/info/cmdline.txt"
rootcmd 'ls -l /dev/block/by-name' 2>/dev/null | tr -d '\r' > "$OUT/info/by-name.txt"
rootcmd 'cat /proc/mounts' 2>/dev/null | tr -d '\r' > "$OUT/info/mounts.txt"

# ---- partition list --------------------------------------------------------
BYNAME="/dev/block/by-name"
PARTS="$(rootcmd "ls -1 $BYNAME" 2>/dev/null | tr -d '\r' | sed '/^$/d')"
if [ -z "$PARTS" ]; then
	# some Exynos kernels expose it under platform/<soc>/by-name
	ALT="$(rootcmd 'ls -d /dev/block/platform/*/by-name' 2>/dev/null | tr -d '\r' | head -n1)"
	if [ -n "$ALT" ]; then
		BYNAME="$ALT"
		PARTS="$(rootcmd "ls -1 $BYNAME" 2>/dev/null | tr -d '\r' | sed '/^$/d')"
	fi
fi
[ -n "$PARTS" ] || die "Could not list $BYNAME on the phone."
log "Found $(echo "$PARTS" | wc -l) partitions in $BYNAME"

# Filter, and gather sizes
declare -A SIZE
LIST=()
TOTAL=0
while IFS= read -r p; do
	[ -n "$p" ] || continue
	if [ "$p" = "userdata" ] && [ "$WITH_USERDATA" -eq 0 ]; then
		log "Skipping userdata (use --with-userdata for a raw dump; /sdcard is copied separately)"
		continue
	fi
	if [ "$p" = "mmcblk0" ] && [ "$WITH_DISK" -eq 0 ]; then
		log "Skipping mmcblk0 (whole disk; every partition is dumped separately - use --with-disk to include)"
		continue
	fi
	case "$SKIP_LIST" in *",$p,"*) log "Skipping $p (--skip)"; continue ;; esac
	if [ "$p" = "super" ] && [ "$SKIP_SUPER" -eq 1 ]; then
		log "Skipping super (--skip-super)"
		continue
	fi
	sz="$(rootcmd "blockdev --getsize64 $BYNAME/$p 2>/dev/null" | tr -d '\r\n ')"
	case "$sz" in ''|*[!0-9]*) sz=0 ;; esac
	SIZE[$p]="$sz"
	TOTAL=$((TOTAL + sz))
	LIST+=("$p")
done <<< "$PARTS"

log "Partitions to dump: ${#LIST[@]}, total $(human "$TOTAL")"

AVAIL_KB="$(df -Pk "$OUT" | awk 'NR==2{print $4}')"
AVAIL=$((AVAIL_KB * 1024))
if [ "$AVAIL" -lt "$((TOTAL + TOTAL / 20))" ]; then
	die "Not enough free space on this PC: need ~$(human "$TOTAL"), have $(human "$AVAIL").
    Try --skip-super or choose another disk with -o."
fi

# ---- progress bar ----------------------------------------------------------
BAR_W=28
DONE_BYTES=0          # bytes of fully finished partitions
T0_ALL=${EPOCHREALTIME/./}
LAST_PCT_PRINTED=-1

# progress <label> <idx> <count> <cur> <exp> <start_us>
progress() {
	local label=$1 idx=$2 cnt=$3 cur=$4 exp=$5 t0=$6
	local now=${EPOCHREALTIME/./} el speed=0 eta="--:--" pct=0 filled bar="" tot_pct=0 all=$((DONE_BYTES + cur))
	el=$((now - t0)); [ "$el" -lt 1 ] && el=1
	speed=$((cur * 1000000 / el))
	[ "$exp" -gt 0 ] && pct=$((cur * 100 / exp)) && [ "$pct" -gt 100 ] && pct=100
	[ "$TOTAL" -gt 0 ] && tot_pct=$((all * 100 / TOTAL)) && [ "$tot_pct" -gt 100 ] && tot_pct=100
	if [ "$speed" -gt 0 ] && [ "$TOTAL" -gt 0 ]; then
		local rem=$((TOTAL - all)); [ "$rem" -lt 0 ] && rem=0
		eta="$(mmss $((rem / speed)))"
	fi

	if [ "$IS_TTY" -eq 1 ]; then
		filled=$((pct * BAR_W / 100))
		bar="$(printf '%*s' "$filled" '' | tr ' ' '#')$(printf '%*s' $((BAR_W - filled)) '' | tr ' ' '-')"
		if [ "$exp" -gt 0 ]; then
			printf '\r\033[K%-14.14s [%s] %3d%% %7s/s  all %3d%%  ETA %s  (%d/%d)' \
				"$label" "$bar" "$pct" "$(human "$speed")" "$tot_pct" "$eta" "$idx" "$cnt" >&3
		else
			printf '\r\033[K%-14.14s %9s copied  %7s/s  (%d/%d)' \
				"$label" "$(human "$cur")" "$(human "$speed")" "$idx" "$cnt" >&3
		fi
	else
		# not a terminal (piped/cron): one plain line per 25%
		local step=$((pct / 25))
		if [ "$step" -ne "$LAST_PCT_PRINTED" ] && [ "$exp" -gt 0 ]; then
			LAST_PCT_PRINTED=$step
			printf '%s: %d%% (%s/s, all %d%%)\n' "$label" "$pct" "$(human "$speed")" "$tot_pct" >&3
		fi
	fi
}

# dump_partition <name> <expected_size> <outfile> <idx> <count>
# Streams phone -> PC and draws progress by polling the output file size.
dump_partition() {
	local p=$1 exp=$2 f=$3 idx=$4 cnt=$5 cur=0 rc t0
	t0=${EPOCHREALTIME/./}
	LAST_PCT_PRINTED=-1
	rootcmd "dd if=$BYNAME/$p bs=4M 2>/dev/null" > "$f" &
	CUR_PID=$!
	local key="" skipped=0
	while kill -0 "$CUR_PID" 2>/dev/null; do
		cur=$(stat -c %s "$f" 2>/dev/null || echo 0)
		progress "$p" "$idx" "$cnt" "$cur" "$exp" "$t0"
		if [ "$IS_TTY" -eq 1 ] && read -rsn1 -t 0.2 key 2>/dev/null && [ "$key" = "s" ]; then
			kill "$CUR_PID" 2>/dev/null; skipped=1; break
		fi
		[ "$IS_TTY" -eq 1 ] || sleep 0.2
	done
	wait "$CUR_PID" 2>/dev/null; rc=$?
	if [ "$skipped" -eq 1 ]; then
		CUR_PID=""; rm -f "$f"; DUMP_BYTES=0; DUMP_SECS=0
		[ "$IS_TTY" -eq 1 ] && printf '\r\033[K' >&3
		return 99
	fi
	CUR_PID=""
	cur=$(stat -c %s "$f" 2>/dev/null || echo 0)
	progress "$p" "$idx" "$cnt" "$cur" "$exp" "$t0"
	[ "$IS_TTY" -eq 1 ] && printf '\r\033[K' >&3
	DUMP_BYTES=$cur
	DUMP_SECS=$(( (${EPOCHREALTIME/./} - t0) / 1000000 ))
	return $rc
}

# ---- dump ------------------------------------------------------------------
FAILED=()
SHAFILE="$OUT/SHA256SUMS"
[ "$CHECKSUMS" -eq 1 ] && : > "$SHAFILE"
i=0
for p in "${LIST[@]}"; do
	i=$((i + 1))
	f="$OUT/partitions/$p.img"
	exp="${SIZE[$p]}"
	log "[$i/${#LIST[@]}] $p ($(human "$exp"))  - press s to skip"
	DUMP_BYTES=0; DUMP_SECS=0
	dump_partition "$p" "$exp" "$f" "$i" "${#LIST[@]}"
	if [ $? -eq 99 ]; then
		warn "$p: skipped by you (s)"; DONE_BYTES=$((DONE_BYTES + exp)); continue
	fi
	got=$DUMP_BYTES
	if [ "$got" -eq 0 ]; then
		warn "$p: empty/unreadable, skipping"
		rm -f "$f"; FAILED+=("$p"); continue
	fi
	if [ "$exp" -gt 0 ] && [ "$got" -ne "$exp" ]; then
		warn "$p: size mismatch (got $got, expected $exp)"
		FAILED+=("$p")
		DONE_BYTES=$((DONE_BYTES + got))
		continue
	fi
	DONE_BYTES=$((DONE_BYTES + got))
	log "    done: $(human "$got") in $(mmss "$DUMP_SECS")"
	if [ "$CHECKSUMS" -eq 1 ]; then
		h="$(sha256sum "$f" | awk '{print $1}')"
		echo "$h  partitions/$p.img" >> "$SHAFILE"
		if [ "$VERIFY" -eq 1 ]; then
			dh="$(rootcmd "sha256sum $BYNAME/$p 2>/dev/null" | tr -d '\r' | awk '{print $1}')"
			if [ "$dh" != "$h" ]; then
				warn "$p: hash differs from the phone's copy!"
				FAILED+=("$p")
			else
				log "    verified against phone"
			fi
		fi
	fi
done

# ---- internal storage ------------------------------------------------------
if [ "$DO_SDCARD" -eq 1 ]; then
	log "Copying /sdcard (internal storage); adb shows its own progress:"
	mkdir -p "$OUT/sdcard"
	"${ADB[@]}" pull -a /sdcard/. "$OUT/sdcard/" >&3 2>&1 \
		|| warn "adb pull of /sdcard had errors"
	log "/sdcard copy finished"
fi

# ---- restore notes ---------------------------------------------------------
{
	echo "Restore notes (SM-A145F backup)"
	echo "================================"
	if [ "$CHECKSUMS" -eq 1 ]; then
		echo "Verify the files first:   sha256sum -c SHA256SUMS"
		echo
	fi
	cat <<'EOF'
Flash a single partition back (phone rooted or in recovery with adb root):
    adb push partitions/boot.img /data/local/tmp/boot.img
    adb shell su -c "dd if=/data/local/tmp/boot.img of=/dev/block/by-name/boot bs=4M"

!! Only restore what you need. Never write efs/sec_efs/modem partitions back
   unless your IMEI/baseband is actually broken - and never to a different phone.
!! Last resort: flash official stock firmware with Odin / Heimdall from Download
   mode (hold Vol Up + Vol Down while plugging in USB).
EOF
} > "$OUT/RESTORE.txt"

# ---- summary ---------------------------------------------------------------
ELAPSED=$(( (${EPOCHREALTIME/./} - T0_ALL) / 1000000 ))
log "Finished in $(mmss "$ELAPSED"): $(human "$DONE_BYTES") copied to $(readlink -f "$OUT")"
if [ ${#FAILED[@]} -gt 0 ]; then
	warn "Problems with: ${FAILED[*]}"
	warn "Make sure these are readable as root; rerun with --verify to double-check."
	exit 2
fi
log "All partitions copied. Keep this folder safe, especially partitions/efs.img"
log "and partitions/sec_efs.img (your IMEI)."
