//
//  main.c
//  Calc
//
//  Created by Jsday on 27/09/16.
//  Copyright © 2016 Reseaux informatique. All rights reserved.
//

#include <stdio.h>
#include <unistd.h> //getopt
#include <ctype.h> //isdigit
#include <stdlib.h> //abort
#include <string.h> //strchr

//header
int add(double *cval, char *aarg);
int sub(double *cval, char *sarg);
int multiplication(double *cval, char *marg);
int divide(double *cval, char *darg);
double increment(double cval);
double decrement(double cval);
int is_digit(char *arg);
int hex_or_digit(char *arg);

//main
int main(int argc, char **argv) {
    
    int o, accurancy = 0;
    double cval = 0;
    
    if (argc < 2) {// if there is no argument
        fprintf(stdout, "%.*f\n", accurancy, cval);
        return EXIT_SUCCESS;
    }
    
    while ((o = getopt (argc, argv, "a:s:m:d:ID")) != -1)
        switch (o)
    {
        case 'a': // ADD operation
            if(add(&cval, optarg) != 0){
                return EXIT_FAILURE;
            }
            //if the next element of the argv[] is not in the string
            if (strchr("asmdID", optind) == NULL) {
                if (argv[optind]!=NULL) {
                    accurancy = atoi(argv[optind]);
                }
            }
            break;
            
        case 's': // SUB operation
            if(sub(&cval, optarg) != 0){
                return EXIT_FAILURE;
            }
            //if the next element of the argv[] is not in the string
            if (strchr("asmdID", optind) == NULL) {
                if (argv[optind]!=NULL) {
                    accurancy = atoi(argv[optind]);
                }
            }
            break;
            
        case 'm': // MUL operation
            if(multiplication(&cval, optarg) != 0){
                return EXIT_FAILURE;
            }
            //if the next element of the argv[] is not in the string
            if (strchr("asmdID", optind) == NULL) {
                if (argv[optind]!=NULL) {
                    accurancy = atoi(argv[optind]);
                }
            }
            break;
            
        case 'd': // DIV operation
            if(divide(&cval, optarg) != 0){
                return EXIT_FAILURE;
            }
            //if the next element of the argv[] is not in the string
            if (strchr("asmdID", optind) == NULL) {
                if (argv[optind]!=NULL) {
                    accurancy = atoi(argv[optind]);
                }
            }
            break;
            
        case 'I': // INC operation
            cval = increment(cval);
            //if the next element of the argv[] is not in the string
            if (strchr("asmdID", optind) == NULL) {
                if (argv[optind]!=NULL) {
                    accurancy = atoi(argv[optind]);
                }
            }
            break;
            
        case 'D': // DEC operation
            cval = decrement(cval);
            //if the next element of the argv[] is not in the string
            if (strchr("asmdID", optind) == NULL) {
                if (argv[optind]!=NULL) {
                    accurancy = atoi(argv[optind]);
                }
            }
            break;
            
        case '?':
            fprintf (stderr, "Unknown option '-%c'.\n", optopt);
            return EXIT_FAILURE;
        default:
            fprintf (stderr, "Unknown option '-%c'.\n", optopt);
            return EXIT_FAILURE;
    }
    
    fprintf(stdout, "%.*f\n", accurancy, cval);
    return EXIT_SUCCESS;
}

/**
 * check if arg is digit
 * @pre : arg != NULL
 * @return 0 if digit otherwise 1
 */
int is_digit(char *arg){
    char *digit = ".0123456789"; // set of digit
    for (size_t i=0; i<strlen(arg); i++) {
        if (strchr(digit, arg[i]) == NULL) {
            return 1; // is not a digit
        }
    }
    return 0;
}

/**
 * check if arg is numerique
 * @pre : arg != NULL
 * @return 0 if hex, 2 if digit otherwise 1
 */
int hex_or_digit(char *arg){
    char *hex =".0123456789aAbBcCdDeEfF"; // set of hexadecimal
    
    if (arg[0]=='0' && arg[1]=='x') { // check if it is hex
        for (size_t i=2; i<strlen(arg); i++) {
            if (strchr(hex, arg[i]) == NULL) {
                return 1; // is not a digit or hex
            }
        }
        return 0; // is hex
    }else if(is_digit(arg) == 0){
        return 2; // is a digit
    }
    else{
        return 1;
    }
}

/**
 * add aarg to cval
 * @pre : aarg != NULL
 * @return 0 if success, 1 if not numerique otherwise -1
 */
int add(double *cval, char *aarg){
    int digit = hex_or_digit(aarg);
    switch (digit) {
        case 0: // hex number
            *cval = *cval + strtol(aarg, NULL, 0);
            return 0;
        case 2: // digit number
            *cval = *cval + strtod(aarg, NULL);
            return 0;
        case 1: // not digit or hex
            fprintf (stderr, "(add) Not numerique : %s\n", aarg);
            return 1;
        default:
            fprintf (stderr, "error with add function\n");
            return -1;
    }
}

/**
 * sub aarg to cval
 * @pre : sarg != NULL
 * @return 0 if success, 1 if not numerique otherwise -1
 */
int sub(double *cval, char *sarg){
    int digit = hex_or_digit(sarg);
    switch (digit) {
        case 0: // hex number
            *cval = *cval - strtol(sarg, NULL, 0);
            return 0;
        case 2: // digit number
            *cval = *cval - strtod(sarg, NULL);
            return 0;
        case 1: // not digit or hex
            fprintf (stderr, "(sub) Not numerique : %s\n", sarg);
            return 1;
        default:
            fprintf (stderr, "error with sub function\n");
            return -1;
    }
}

/**
 * multiply cval by marg
 * @pre : marg != NULL
 * @return 0 if success, 1 if not numerique otherwise -1
 */
int multiplication(double *cval, char *marg){
    int digit = hex_or_digit(marg);
    switch (digit) {
        case 0: // hex number
            *cval = *cval * strtol(marg, NULL, 0);
            return 0;
        case 2: // digit number
            *cval = *cval * strtod(marg, NULL);
            return 0;
        case 1: // not digit or hex
            fprintf (stderr, "(multiplication) Not numerique : %s\n", marg);
            return 1;
        default:
            fprintf (stderr, "error with multiplication function");
            return -1;
    }
}

/**
 * divide cval by darg
 * @pre : darg != NULL
 * @return 0 if success, 1 if not numerique otherwise -1
 */
int divide(double *cval, char *darg){
    long arg = 0;
    int digit = hex_or_digit(darg);
    switch (digit) {
        case 0: // hex number
            arg = strtol(darg, NULL, 0);
            if (arg == 0) {
                fprintf (stderr, "divide by : %s\n", darg);
                return 1;
            }
            *cval = *cval / strtol(darg, NULL, 0);
            return 0;
        case 2: // digit number
            arg = strtod(darg, NULL);
            if (arg == 0) {
                fprintf (stderr, "divide by : %s\n", darg);
                return 1;
            }
            *cval = *cval / strtod(darg, NULL);
            return 0;
        case 1: // not digit or hex
            fprintf (stderr, "(divide) Not numerique : %s\n", darg);
            return 1;
        default:
            fprintf (stderr, "error with divide function");
            return -1;
    }
}

/**
 * increment cval by 1
 * @return : cval + 1
 */
double increment(double cval){
    return cval + 1;
}

/**
 * deccrement cval by 1
 * @return : cval - 1
 */
double decrement(double cval){
    return cval - 1;
}
