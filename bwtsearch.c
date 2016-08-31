//
// Created by Chenyu on 11/04/2016.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

void WriteOut(char *adds, char *bwtadds);
int * BackwardSearch(FILE *fp, FILE *indexfp,int *index, char *p);
char GetCharacter(int position, FILE * fp);
int BackwardDecoding (FILE *fp, FILE *indexfp, int *index, int first, int last, int mode);
int Occurance(char c, int pos, FILE *fp, FILE * indexfp);
void ReadIn(char *adds, int * index);


int main(int argc, char *argv[]) {


    char *Nmode = "-n";
    char *Amode = "-a";
    char *Rmode = "-r";
    int index[128];
    int i = 0;

    for (i = 0; i <128 ; ++i) {
        index[i] = 0;
    }

    int flag = access(argv[3], F_OK);
    // if index doesn't file exists
    if (flag != 0) {
        WriteOut(argv[3], argv[2]);
    }
    FILE *fp = fopen(argv[2], "rw");
    FILE *indexfp = fopen(argv[3], "rb");
    // "-n"
    ReadIn(argv[2],index);
    if (strcmp(argv[1], Nmode) == 0) {
        int * result;
        result = BackwardSearch(fp,indexfp, index, argv[4]);
        if (result != NULL) {
            printf("%d\n", result[1] - result[0] + 1);

        }
    }
//     "-a"
    if (strcmp(argv[1], Amode) == 0)
    {
        int * result;
        result = BackwardSearch(fp, indexfp,index, argv[4]);
        if (result != NULL) {
            BackwardDecoding(fp,indexfp,index,result[0],result[1], 1);
        }
    }
    // "-r"
    if (strcmp(argv[1], Rmode) == 0)
    {
        int * result;
        result = BackwardSearch(fp, indexfp,index, argv[4]);
        if (result != NULL) {
            BackwardDecoding(fp,indexfp,index,result[0],result[1], 2);
        }
    }
    fclose(fp);
    fclose(indexfp);
    return 0;
}



void reverse(char *string)
{
    int length, c;
    char *begin, *end, temp;

    length = strlen(string);
    begin  = string;
    end    = string;

    for (c = 0; c < length - 1; c++)
        end++;

    for (c = 0; c < length/2; c++)
    {
        temp   = *end;
        *end   = *begin;
        *begin = temp;

        begin++;
        end--;
    }
}


void ReadIn(char *adds, int * index) {
    FILE *fp;
    size_t bytes_read;
    size_t nbytes = 1000;
    char *my_string;
    int i = 0;
    fp = fopen(adds, "r");
    int sum = 0;
    if(fp == NULL) {
        printf("opening file faile");
    }
    while(1) {
        my_string = (char *) malloc (nbytes + 1);
        bytes_read = getline(&my_string, &nbytes, fp);

        if (bytes_read == -1) {
            break;
        }
        else {
            for (i = 0; i < bytes_read; ++i) {
                index[my_string[i] + 1]+=1;
            }
        }
        free(my_string);
    }
    sum = index[0];
    for (i = 0; i < 128; i++) {
        index[i] = sum + index[i];
        sum = index[i];
    }
    fclose(fp);

}

void WriteOut(char *adds, char *bwtadds) {

    FILE *fp;
    fp = fopen(adds, "wb");
    // count every 1000 letter
    FILE * bwtfp;
    int i = 0;
    int asc[127];

    for (i = 0; i < 127; ++i) {
        asc[i] = 0;
    }
    bwtfp = fopen(bwtadds,"r");
    char ch = ' ';
    int count = 0;
    while ((ch = fgetc(bwtfp)) != EOF) {
        count++;
        asc[ch] += 1;
        if (count==1000) {
            for (i=0; i<127 ;i++) {
                fwrite(&(asc[i]), sizeof(asc[i]), 1, fp);
            }
            count = 0;
        }
    }
    fflush(fp);
    fclose(fp);
    fclose(bwtfp);
}


int Occurance(char c, int pos, FILE *fp, FILE * indexfp) {


    int bucketnum  = pos/1000; //figure out the bucket number
    int rank = pos - bucketnum * 1000;
    int count = 0;
    char ch = ' ';

    if (rank != 0){
        // locate the position in bwt
        fseek(fp, bucketnum*1000 ,SEEK_SET);
        while ((ch = fgetc(fp)) != EOF) {
            if (ch == c) {
                count += 1;
            }
            rank -= 1;
            if (rank <= 0) {
                break;
            }
        }
    }
    if (bucketnum == 0) {
        return count;
    }
    // count from index
    int num = 0;
    fseek(indexfp, (bucketnum-1)*127*4 + c*4, SEEK_SET);
    fread(&num,sizeof(int),1,indexfp);
    count = count+num;

    return count;
}

int * BackwardSearch(FILE *fp, FILE *indexfp, int *index, char *p) {
    fseek(fp,0,SEEK_SET);
    fseek(indexfp,0,SEEK_SET);
    int *result;
    result = (int *)malloc(2);
    int i = strlen(p);
    char c = p[i - 1];
    int first = index[c] + 1;
    int last =  index[c + 1];

    while (first <= last && i>=2)
    {
        c = p[i - 1 - 1];
        int a = index[c];
        int b = Occurance(c, first - 1, fp, indexfp);
        first = a + b + 1;
        int d = index[c];
        int e = Occurance(c, last,fp,indexfp);
        last = d + e;
        i = i - 1;
    }
    if (last < first) {
        printf("Can't find any match\n");
        return NULL;
    }
    else {
        result[0] = first;
        result[1] = last;
        return result;
    }

}

// return character by given position
char GetCharacter(int position, FILE * fp) {

    fseek(fp,position-1,SEEK_SET);
    return fgetc(fp);

}
// Use for -a mode, using backward decoding find offset number,
// argument 'c' STAND FOR THE FIRST LETTER OF SEARCH PATTERN
// mode 1 for '-a', mode 2 for '-r'
int BackwardDecoding (FILE *fp, FILE *indexfp, int *index, int first, int last, int mode) {

    fseek(fp,0,SEEK_SET);
    fseek(indexfp,0,SEEK_SET);
    int * offset;
    int size = last - first + 1;
    offset = (int*)malloc(sizeof(int)*(size));
    char temp = ' ';
    int position = 0;
    int flag = 0;
    int i = 0;
    int sum = 0;
    int j;
    for (i = 0; i < size; ++i) {
        offset[i] = 0;
    }
    i = 0;
    while(first <= last)
    {
        position = first;
        while (temp != '[') {

            temp = GetCharacter(position,fp);
            if(flag == 1 && temp != '[') {
                sum = (temp - '0') * pow(10, i) + sum;
                i++;
            }
            int g = index[temp];
//            clock_t t;
//            t = clock();
            int h = Occurance(temp, position, fp, indexfp);
//            t = clock() - t;
//            double time_taken = ((double)t)/CLOCKS_PER_SEC; // in secon
//            printf("%f\n", time_taken);
            position = g + h;
            if (temp == ']') {
                flag = 1;
            }

        }
        for (i = 0; i < size ; ++i) {
            if (sum == offset[i]) {
                break;
            }
            if (offset[i] == 0)
            {
                offset[i] = sum;
                break;
            }
        }

        temp = ' ';
        first += 1;
        flag = 0;
        i = 0;
        sum = 0;
    }
    int l = 0;
    for (i = 0; i < size; ++i)
    {
        for (j = i + 1; j < size; ++j)
        {
            if (offset[i] > offset[j])
            {
                l =  offset[i];
                offset[i] = offset[j];
                offset[j] = l;
            }
        }
    }
    if (mode == 1) {
        for (i = 0; i < size; ++i) {
            if (offset[i] == 0) {
                continue;
            }
            printf("[%d]\n", offset[i]);
        }
    }
    if (mode == 2) {
        int count = 0;
        for (i = 0; i < size; ++i) {
            if (offset[i] == 0) {
                continue;
            }
            count += 1;
        }
        printf("%d\n",count);
    }
    return 0;

}

