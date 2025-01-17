/*******************************************************************************
* Title                 :   The First Assembler Pass
* Filename              :   pass_one.c
* Author                :   Itai Kimelman
* Version               :   1.5.4
*******************************************************************************/
/** \file pass_one.c
 * \brief This module performs the 1st assembler pass
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assembler.h>
/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/
#define MEMORY_MAX  pow(2,25)-1
/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
#define memory_lim(X)   ((X)>=0 && (X)<=MEMORY_MAX)? 1:0
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
int err1; /*indicates if there's an error in the current file*/
int err_ln; /*indicates if there's an error in the current line*/
unsigned long ICF; /*the final value of IC*/
unsigned long DC,DCF; /*the current and final value of DC respectfully*/
extern int data_exists;
/******************************************************************************
* Function Definitions
*******************************************************************************/
/******************************************************************************
* Function : pass_one_error(char *file_name, unsigned long num_ln)
*//**
* \section Description: this function is used when an error in input occurs during the 1st pass.
*                       it turns on the err1,err_ln flags, and prints out the name of the file and line number,
*                       so the user will know where the error was found.
*
* \param  		file_name - the name of the current file
* \param        num_ln - the current line number
*******************************************************************************/
void pass_one_error(char* file_name,unsigned long num_ln) {
    err1 = STATUS_ERR;
    err_ln = STATUS_ERR;
    fprintf(stderr,"[%s | %lu]\n",file_name,num_ln);
}

/******************************************************************************
* Function : pass_one(char *file_name)
*//**
* \section Description: this function performs the 1st assembler pass on the current file.
*                       it follows the algorithm mentioned below.
* \param  		file_name - the name of the current file
* \return       STATUS_OK if no error was found. otherwise: STATUS_ERR
* \note
 * the algorithm for the 1st assembler pass is as follows:
 * 1. initialize IC = 100, DC = 0
 * 2. read the next line. if the file has ended, go to step 17
 * 3. if it's a comment line or an empty line, go to step 2
 * 4. is the 1st field in the line a label? if not, go to step 6
 * 5. turn on the label flag
 * 6. is this a data directive? if not go to step 9
 * 7. if there is a label, add it to the symbol table(if the label already exists, report an error) with the attribute "data"
 * and value DC
 * 8. identify the data directive and code the data requested accordingly into the data image table with value DC.
 * add the right amount to DC. go to step 2
 * 9. is this an .entry or .extern directive? if not go to 12
 * 10. if this is an .entry directive go to step 2
 * 11. if this is an .extern directive, add the label that shows up as the operand of this directive
 * to the symbol table with the attribute "external" and value 0 (if the label already exists, report an error)
 * 12. this is an order line. if there is a label, add it to the symbol table(if the label already exists, report an error) with the attribute "code"
 * and value IC
 * 13. look for the order in the opcode table. if it does not exist, report an error
 * 14. analyze the operand structure of the order. if an error occurs, report it
 * 15. code the order to the binary image of the code as much as possible with value IC.
 * 16. update IC+=4 and go to step 2
 * 17. the file has been read entirely. if there was an error, stop here (there will not be a 2nd pass or output files).
 * 18. save the final value of IC,DC into ICF,DCF accordingly. they will be used to build the output files
 * 19. update the value of every symbol with attribute data by adding ICF to its value
 * 20. update the data image table by adding ICF to each value
 * 21. return FALSE to main (begin the 2nd assembler pass) (no error was found)
*******************************************************************************/
int pass_one(char *file_name) {
    unsigned long num_ln = 0;
    long unsigned IC;
    int label_flag; 			/*indicates if there is a label in the current line*/
    char line[MAX_LINE+1];		/* the current line */
    char *pos = NULL;			/* pointer in current line */
    FILE *curr_file; 			/*pointer to file */
    char label[MAX_LINE+1];		/*saves label (if there is one) */
    /*step 1:*/
    IC = 100;
    DC = 0;
    err1 = STATUS_OK;

    if((curr_file=fopen(file_name,"r"))==NULL) {
        fprintf(stderr,"error while opening file");
        err1 = STATUS_ERR;
        return err1;
    }
    if((fseek(curr_file,0,SEEK_SET)) != 0) {
        fprintf(stderr,"error trying to pass on the file %s", file_name);
        err1 = STATUS_ERR;
        return err1;
    }

    while(TRUE){
        num_ln++;
        err_ln = FALSE;
        label_flag = FALSE;
        /*step 2:*/
        if(read_line(curr_file, line) == FALSE)
            break;
        /*checking if the line length is above the maximum allowed*/
        if(length_check(line) == FALSE) {
            pass_one_error(file_name,num_ln);
            continue;
        }

        pos = line;

        while(spaceln(*pos))
            pos++;
        /*step 3:*/
        if(meaningless(pos))
            continue;

        /*step 4:*/
        if(start_label(pos)) {
            scan_label(pos, label); /*keeping the label for later*/
            /*step 5:*/
            label_flag=TRUE;
            pos+= next_op(pos,FALSE); /*skipping the label so that we won't try to parse it as something else*/
        }
        /*step 6:*/
        if(is_data(pos)) {
            /*checking the structure of the directive:*/
            if(compatible_args(pos) == FALSE) {
                pass_one_error(file_name,num_ln);
            }
            /*step 7:*/
            if(err_ln == FALSE) {
                if(label_flag) {
                    if(add_symbol(DC, label, DATA,FALSE) == FALSE) {
                        pass_one_error(file_name,num_ln);
                    }
                }
            }
            /*step 8:*/
            if(err_ln == FALSE) {
                data_to_info(pos);
            }
        } else {
            /*step 9:*/
            if(ent_ext(pos)) {
                /*steps 10,11:*/
                if(ent_ext(pos) == EXTERN) {
                    if(check_ent_ext(pos) == FALSE) {
                        pass_one_error(file_name,num_ln);
                    } else {
                        pos+= next_op(pos, FALSE);
                        scan_label(pos, label);
                        if (err_ln == FALSE) {
                            if (add_symbol(0, label, EXTERNAL, FALSE) == FALSE)
                                pass_one_error(file_name, num_ln);
                        }
                    }
                }
            } else {
                /*step 12:*/
                if(label_flag == TRUE) {
                    if(add_symbol(IC, label, CODE,FALSE) == FALSE)
                        pass_one_error(file_name,num_ln);
                }
                /*steps 13 and 14:*/
                if(err_ln == FALSE) {
                    if(order_structure(pos) == FALSE)
                        pass_one_error(file_name,num_ln);
                }
                /*step 15:*/
                if(err_ln == FALSE) {
                    cmd_to_info(pos,IC);
                }
                IC+=WORD; /*step 16*/
            }
        }
    }
    /*step 17:*/
    if(err1 == STATUS_ERR) {
        return err1;
    }
    /*step 18:*/
    ICF = IC;
    DCF = DC;
    /*check memory here:*/
    if(!memory_lim(ICF+DCF)) {
        fprintf(stderr,"error: this file requests more storage than this computer has (it has 2^25 bytes of storage) ");
        err1 = STATUS_ERR;
        return err1;
    }
    /*step 19:*/
    if(data_exists)
        update_data_img(ICF);
    /*step 20:*/
    update_symbol_table(ICF);
    /*step 21*/
    return STATUS_OK;
}

/*************** END OF FUNCTIONS ***************************************************************************/
