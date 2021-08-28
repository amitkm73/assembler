/*******************************************************************************
* Title                 :   Main Program
* Filename              :   main.c
* Author                :   Itai Kimelman
* Version               :   1.4.2
*******************************************************************************/
/** \file main.c
 * \brief This module contains the main function of the assembler
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include "assembler.h"
/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : main(int argc, char **argv)
*//**
* \section Description Description: The main function of the assembler.
* it takes each input file (as an argument in argv), and tries to translate it into machine code.
* if an error occurs in one output file, the assembler will still run perfectly on the rest.
*
* \param  		argc - the number of arguments
* \param        argv - the arguments
*
* \return 		0 if there is no error in every file given
*
*******************************************************************************/
int main(int argc, char **argv) {
    int i, err, err_total;
    char *curr_file;
	err_total = 0;
    if(num_files(argc) == 0) return 1;
    for(i = 1; i< argc; i++) {
        if ((curr_file = filename(argv[i])) != NULL) {
				initialize_tables();
				mem_allocate();
				err = pass_one(curr_file);
				if (err) {
					err_total += err;
					mem_deallocate();
					continue;
				}
				err = pass_two(curr_file);
				if (err) {
					err_total += err;
					mem_deallocate();
					continue;
				}
				output(curr_file);
				mem_deallocate();
        }
    }
    return err_total;
}

/*************** END OF FUNCTIONS ***************************************************************************/
