#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>

#define TRUE  1
#define FALSE 0

typedef struct
{
    char word[10];
}wordtype;

FILE* dict;
long seek[26];
int word_length,chain_length,change_map,tried_map;
int easy = TRUE,fail,success=FALSE,found=FALSE;
char word[10];
wordtype words[10];
wordtype *current_word, *last_word;

int findchain(void);
wordtype* findword(wordtype* current_word, int pos);
wordtype* findword2(wordtype* current_word, int pos);
int isindict(char* word, int length);

int main(int argc, char* argv[])
{
    char dict_word[50],name[20];
    int count=0,chosen,tries=0;//,changed,pos;

    int i,j;

    printf("Generate change-one-letter word puzzle\n");

    srand(time(0));

    //printf("argc = %d",argc);

    if (argc == 2)  //word supplied on cmdline
    {
        strncpy(word, (char*)argv[1], strlen((char*)argv[1])+1);    //copy cmdline arg to word. +1 for null terminator.
        word_length = strlen(word);
        chain_length = word_length;
        //printf("length = %d",word_length);
        //printf("word = %s",word);

        sprintf(name,"dict%d.txt",word_length);
        dict = fopen(name,"r");
    }
    else
    {
        if (argc == 3) //wl/cl on cmdline
        {
            word_length = atoi(argv[1]);
            chain_length = atoi(argv[2]);
            printf("WL:%d CL:%d\n",word_length,chain_length);
        }

        else
        {
            word_length = 4+rand()%3;
            chain_length = word_length;
            //printf("length = %d",word_length);
        }
        sprintf(name,"dict%d.txt",word_length);
        dict = fopen(name,"r");
        if (dict == NULL)
        {
            printf("Failed to open dictionary file:%s\n",name);
            return 0;
        }

        while (fgets(dict_word,50,dict) != NULL) count++;        //count words
        rewind(dict);
    }

    while (!success)
    {
        if (argc != 2)
        {
            chosen = rand()%count;                              //pick random word
            //printf("count = %d, chosen = %d\n",count,chosen);
            rewind(dict);
            for (i=0 ; i<chosen ; i++)
                fgets(word,50,dict);

            word[strcspn(word, "\n")] = 0;      //trim cr/lf
            //printf("word = %s\n",word);
            rewind(dict);
        }

        //work out seek positions for each letter
        for (i=0 ; i<26 ; i++)
        {
            while(1)
            {
                seek[i] = ftell(dict);          //mark position
                fgets(dict_word,50,dict);       //read word
                if (dict_word[0] == i + 'a')    //if it starts with the right letter, break so we increment i
                {
                     //printf("%s",dict_word);
                     break;
                }
                if (dict_word[0] > i + 'a') //we've 'skipped ahead, meaning there are no words starting with this letter
                {
                    break;                  //break anyway; we'll never have a word starting with this letter in a chain,
                }                           // if user puts one in, we won't find it regardless.
            }
        }

        printf ("\nSTART:%s (%d)\n",word,word_length);

        current_word = &words[0];
        strncpy(current_word->word, word,word_length+1);          //put first word into array
/*
        if (searching)
        {
            tries++;
            if (tries == 100)
            {
                //new word
            }
            else
            {
                if (findchain())
                    searching = FALSE;
            }
        }
        if (event)
        {
            //handle touches

            //redraw on timer - draw fn handles start, menu, game screens?

            //when level finished, searching = true.
        }
*/

        //come around here each time we've decided we can't make a chain from the chosen starting word
        for (tries=0 ; tries<100 ; tries++)
        {
            if (findchain())
            {
                success = TRUE;
                break;
            }
        }
    }

    for (j=0 ; j<=chain_length ; j++)
        printf("\n%s",words[j].word);

    fclose(dict);
    return 0;
}

int findchain(void)
{
    int changed,pos;

    current_word = &words[0];
    strncpy(current_word->word, word,word_length+1);        //put first word into array

    //reset variables
    fail = 0;                                               //successive failures to find the next word
    tried_map = 0;                                          //bitmap of positions we've tried
    change_map = 0;                                         //bitmap of which letters have been changed

    //chain_length = word_length;
    //chain_length = 6;               //testing

    if (chain_length > word_length) easy = FALSE;       //otherwise we lock up....
                                                        //in this case, need to check new word against all old words....

    //change as many letters as the chain length
    for (changed=0 ; changed<chain_length ; )//i++)
    {
        //printf(".");
        while(1)        //pick random position to change - don't duplicate
        {
            pos = rand() % word_length;
            if ( ((change_map & (1<<pos)) == 0) && ((tried_map & (1<<pos)) == 0 ) ) break;
        }

        tried_map |= (1<<pos);          //remember we tried this position

        last_word = current_word;
        current_word = findword2(current_word, pos);    //findword(2) takes and returns returns pointer into word list.
                                                        //If unchanged, no new word found
        if (current_word != last_word)
        {
            fail=0;                                     //new word, so reset fail count
            tried_map = 0;                                  //and tried map
            changed++;
        }
        else
            fail++;

        if (changed==chain_length)  return 1;      //changed every letter, so done!

        if (easy)
        {
            if (fail == (word_length - changed))            //consecutive failures == unchanged positions. So give up.
                break;
        }
        else
        {
            if (fail == (word_length))                      //consecutive failures == positions. So give up.
                break;
        }
    }
    return 0;
}

/*
Alternative approach:
Run through entire dictionary once. For each entry, check whether it differs from original word
by exactly one letter. If so, store in array (need big array..... could limit to (say)100 matches
though, I guess)
Pick randomly from array of matches.
Not sure if this is more efficient or not.......
*/
wordtype* findword2(wordtype* current_word, int pos)
{
    wordtype matches[100];    //matches
    wordtype* wordptr;
    int i,j=0,fail,chosen;//,i;
    char dict_word[50];

    rewind(dict);

    while (fgets(dict_word,50,dict) != NULL)        //get word, exit on eof
    {
        //check for only difference in position 'pos'
        fail=0;
        for (i=0 ; i<word_length ; i++)
        {
            if (i==pos)
            {
                if (dict_word[i] == current_word->word[i])
                {
                    fail = 1;
                    break;
                }
            }
            else
            {
                if (dict_word[i] != current_word->word[i])
                {
                    fail = 1;
                    break;
                }
            }
        }
        if (fail == 0)
        {
            //check here for duplication??
            for (wordptr=&words[0] ; wordptr<current_word ; wordptr++)
            {
                if (strncmp(dict_word,wordptr->word,word_length)==0)
                    fail = 1;
            }
            if (fail == 0)
            {
                strncpy(matches[j].word,dict_word,word_length);
                matches[j].word[word_length]=0;
                j++;
            }
        }
    }

    if (j==0) return current_word; //fail

    chosen = rand()%j;                              //pick one of our matches at randonm
    if (easy)
        change_map |= (1<<pos);                    //remember we changed this position
    else
    {

    }

    tried_map = 0;                                                  //reset which positions we tried
    current_word++;                                                 //inc pointer
    strncpy(current_word->word,matches[chosen].word,word_length+1); //copy this word to next word in sequence

    return(current_word);
}

wordtype* findword(wordtype* current_word, int pos)
{
    int letter,letter_map;
    char temp[10];
/*
    //pick random position to change - don't duplicate
    while(1)
    {
        pos = rand() % word_length;
        if ( ((change_map & (1<<pos)) == 0) && ((tried_map & (1<<pos)) == 0 ) ) break;
    }
*/
    tried_map |= (1<<pos);          //remember we tried this position
    letter = word[pos];         //existing letter that we'll replace
    letter_map = 1<<(letter-'a');  //set bit to show us not to use it

    found = FALSE;
    //position decided - try every letter once, but in a random order, so we can generate different answers
    while(!found)
    {
        while ((letter_map & (1<<(letter-'a'))) != 0)      //don't use a letter we've already tried
        {
            letter = (rand() % 26) + 'a';                   //random letter
        }

        letter_map |= (1<<(letter-'a'));                   //remember we've tried this letter
        strncpy(temp,current_word->word,word_length+1);    //make copy with random change
        temp[pos] = letter;

        if (isindict(temp,word_length))                  //check if it's in dictionary
        {
            if (easy)
                change_map |= (1<<pos);                    //remember we changed this position
            else
            {
                //check against rest of chain
            }
            found = TRUE;                               //remember we've got a match
            //strncpy(word,temp,word_length+1);           //copy it back for next attempt

            //fail = 0;                                   //reset fail counter
            tried_map = 0;                                  //reset which positions we tried
            //i++;                                        //inc replaced letter count
            //strncpy(words[i].word, word,word_length+1); //remember next word in sequence
            //if (i==chain_length) success = TRUE;        //changed every letter, so done!
            current_word++;
            strncpy(current_word->word, temp,word_length+1); //remember next word in sequence
        }

        if (letter_map == 0x3ffffff)    //tried every letter
        {
            //fail++;
            break;

        }
    }//end of while(1) i.e. replaced one letter (or given up)

    letter_map = 0;        //so reset which letters were used

    //return i;
    return (current_word);
}

int isindict(char* word, int length)
{
    char dict_word[50];

    fseek(dict,seek[word[0]-'a'],SEEK_SET);         //send dictionary to start of this letter

    while (fgets(dict_word,50,dict) != NULL)        //get word, exit on eof
    {
        if (dict_word[0] != word[0])                //if the first letter doesn't match, we must be past, so skip
            return 0;

        if (strncmp(word, dict_word, length) == 0)  //if the words match we have a valid word
              return 1;
    }
    return 0;

}
