/*
 *--------------------------------------
 * Program Name: SymbolicDerivative
 * Author: Nathan Farlow
 * License: MIT
 * Description: Calculates the symbolic derivative of a yvar
 *--------------------------------------
*/

#include <tice.h>
#include <fileioc.h>

#include <debug.h>

#include <stdlib.h>

#include "../parser.h"
#include "../cas.h"

void printText(int8_t xpos, int8_t ypos, const char *text);

void main(void) {
    ti_var_t y1, y2;

    uint8_t *data;
    uint16_t size;

    os_ClrHome();
    ti_CloseAll();

    y1 = ti_OpenVar(ti_Y1, "r", TI_EQU_TYPE);

    if(y1) {
    	Error error;
    	tokenizer_t t;
    	ast_t *e, *deriv;

    	uint8_t *deriv_data;
    	unsigned deriv_data_size;

		printText(0, 0, "Calculating...");

    	data = ti_GetDataPtr(y1);
    	size = ti_GetSize(y1);

    	error = tokenize(&t, data, size);

    	ti_Close(y1);
    	
    	e = parse(&t, &error);

    	if(error != E_SUCCESS) {
    		printText(0, 1, "Error parsing.");
    		goto err;
    	}

    	deriv = derivative(e, 'X', &error);

    	if(error != E_SUCCESS) {
    		printText(0, 1, "Error calculating derivative.");
    		goto err;
    	}

    	deriv_data = to_binary(deriv, &deriv_data_size, &error);

    	y2 = ti_OpenVar(ti_Y2, "w", TI_EQU_TYPE);
    	ti_Write(deriv_data, deriv_data_size, 1, y2);

    	ti_Close(y2);

    	free(deriv_data);

    } else {
    	printText(0, 4, "Couldn't open.");
    }

    printText(0, 1, "Done.");
err:
    while(!os_GetCSC());
}

void printText(int8_t xpos, int8_t ypos, const char *text) {
    os_SetCursorPos(ypos, xpos);
    os_PutStrFull(text);
}
