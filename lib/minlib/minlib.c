/* 
 * MinLib
 * A very simple implementation of some stdlib functions
 * Copyright (c) 2024 - BotchedRPR
*/



unsigned int strlen(const char *s)
{
    unsigned int count = 0;
    while(*s!='\0')
    {
        count++;
        s++;
    }
    return count;
}