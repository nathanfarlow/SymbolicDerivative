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

#define SIMPLIFY_ITERATIONS 10

void printText(int8_t xpos, int8_t ypos, const char *text);

ast_t *simplify_amount(ast_t *e, unsigned amount) {
	unsigned i;

	if(amount == 0)
		return ast_Copy(e);

	e = simplify(e);

	for(i = 0; i < amount - 1; i++) {
		ast_t *temp = simplify(e);
		free(e);
		e = temp;
	}

	return e;
}

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
    	ast_t *e, *simplified, *deriv, *simplified_deriv;

    	uint8_t *deriv_data;
    	unsigned deriv_data_size;

		printText(0, 0, "Solver by Nathan Farlow");

		printText(0, 2, "Calculating...");

    	data = ti_GetDataPtr(y1);
    	size = ti_GetSize(y1);

    	error = tokenize(&t, data, size);

    	ti_Close(y1);
    	
    	e = parse(&t, &error);

    	if(error != E_SUCCESS) {
    		printText(0, 3, "Error parsing: syntax error.");
    		goto err;
    	}

    	simplified = simplify_amount(e, SIMPLIFY_ITERATIONS);
    	ast_Cleanup(e); //to save on some space

    	deriv = derivative(simplified, 'X', &error);
    	ast_Cleanup(simplified);

    	if(error != E_SUCCESS) {
    		printText(0, 3, "Error calculating derivative.");
    		goto err;
    	}

    	simplified_deriv = simplify_amount(deriv, SIMPLIFY_ITERATIONS);
    	ast_Cleanup(deriv);

    	deriv_data = to_binary(simplified_deriv, &deriv_data_size, &error);
    	ast_Cleanup(simplified_deriv);

    	y2 = ti_OpenVar(ti_Y2, "w", TI_EQU_TYPE);
    	ti_Write(deriv_data, deriv_data_size, 1, y2);

    	ti_Close(y2);

    	free(deriv_data);

    } else {
    	printText(0, 2, "Couldn't open equation.");
    }

    printText(0, 3, "Done.");
err:
    while(!os_GetCSC());
    //TODO:
	//_YEquOnOff                 equ 0021044h
    _OS(asm_ClrTxtShd);
}

void printText(int8_t xpos, int8_t ypos, const char *text) {
    os_SetCursorPos(ypos, xpos);
    os_PutStrFull(text);
}
