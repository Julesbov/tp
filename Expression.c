#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "Dict.h"
#include "Expression.h"

//--------------------------------------------------------------------------
// Do not change code between these lines, or our tests might fail.

typedef enum
{
    NUMBER,
    OPERATOR,
    SYMBOL
} ExprType;

struct Expression_t
{
    ExprType type;
    union
    {
        double num;
        char *symb;
        Operator op;
    } value;
    Expression *left;
    Expression *right;
};

// Constructeurs

Expression *exprSymb(char *var)
{
    Expression *exp = malloc(sizeof(Expression));
    if (exp == NULL)
    {
        fprintf(stderr, "exprSymb: could not allocate memory.\n");
        exit(1);
    }
    exp->type = SYMBOL;
    exp->value.symb = var;
    exp->left = NULL;
    exp->right = NULL;
    return exp;
}

Expression *exprNum(double num)
{
    Expression *exp = malloc(sizeof(Expression));
    if (exp == NULL)
    {
        fprintf(stderr, "exprNum: could not allocate memory.\n");
        exit(1);
    }
    exp->type = NUMBER;
    exp->value.num = num;
    exp->left = NULL;
    exp->right = NULL;
    return exp;
}

Expression *exprOp(Operator op, Expression *left, Expression *right)
{
    if (op != PLUS && op != MINUS && op != TIMES && op != DIV)
    {
        fprintf(stderr, "exprOp: unknown operator.\n");
        exit(1);
    }

    Expression *exp = malloc(sizeof(Expression));
    if (exp == NULL)
    {
        fprintf(stderr, "exprOp: could not allocate memory.\n");
        exit(1);
    }
    exp->type = OPERATOR;
    exp->value.op = op;
    exp->left = left;
    exp->right = right;
    return exp;
}

void exprFree(Expression *exp)
{
    if (exp->type == OPERATOR)
    {
        if (exp->left != NULL)
            exprFree(exp->left);
        if (exp->right != NULL)
            exprFree(exp->right);
    }
    free(exp);
}

static Expression *exprCopy(Expression *exp)
{
    if (exp == NULL)
    {
        fprintf(stderr, "exprCopy: malformed expression.\n");
        exit(1);
    }
    if (exp->type == NUMBER)
        return exprNum(exp->value.num);
    else if (exp->type == SYMBOL)
        return exprSymb(exp->value.symb);
    else if (exp->type == OPERATOR)
        return exprOp(exp->value.op, exprCopy(exp->left), exprCopy(exp->right));
    else
    {
        fprintf(stderr, "exprCopy: unknown node type (%d).\n", exp->type);
        exit(1);
    }
}

//--------------------------------------------------------------------------

// Only enter your code below. You can add additional functions as needed, (but declare
// them as static).

void exprPrint(FILE *fp, Expression *exp)
{
    if(exp->type == NUMBER)
        fprintf(fp, "%lf", exp->value.num);

    else if(exp->type == SYMBOL)
        fprintf(fp, "%s",exp->value.symb);

    else if(exp->type == OPERATOR){
        fprintf(fp, "(");
        exprPrint(fp, exp->left);

        switch(exp->value.op){
            case PLUS : fprintf(fp, "+");
                        break;
            case MINUS : fprintf(fp, "-");
                        break;
            case TIMES : fprintf(fp, "*");
                        break;
            case DIV : fprintf(fp, "/");
                        break;
        }

        exprPrint(fp, exp->right);
        fprintf(fp, ")");
    }
}


double exprEval(Expression *exp, Dict *dict)
{
    double result;

    if(exp->type == NUMBER)
        return exp->value.num;

    else if(exp->type == SYMBOL){
        void* var = dictSearch(dict, exp->value.symb);
        double value = *(double*)var;
        return value;
    }


    else if(exp->type == OPERATOR){

        switch(exp->value.op){
            case PLUS : result = exprEval (exp->left, dict) + exprEval (exp->right, dict);
                        break;
            case MINUS : result = exprEval (exp->left, dict) - exprEval (exp->right, dict);
                        break;
            case TIMES : result = exprEval (exp->left, dict) * exprEval (exp->right, dict); 
                        break;
            case DIV : result = exprEval (exp->left, dict) / exprEval (exp->right, dict);
                        break;
        }
    }

    return result;
}


Expression *exprDerivate(Expression *exp, char *var)
{
    
    if(exp->type == NUMBER)
        return exprNum(0.0);

    else if(exp->type == SYMBOL){
        if(exp->value.symb == var)
            return exprNum(1.0);

        else
            return exprNum(0.0);
    }
    
    switch(exp->value.op){    
        case PLUS : 
            return exprOp(PLUS, exprDerivate(exprCopy(exp->left), var), 
                                    exprDerivate(exprCopy(exp->right), var));

        case MINUS :
            return exprOp(MINUS, exprDerivate(exprCopy(exp->left), var), 
                                    exprDerivate(exprCopy(exp->right), var));

        case TIMES :
            return exprOp(PLUS, exprOp(TIMES, exprDerivate(exprCopy(exp->left), var), exprCopy(exp->right)), 
                                    exprOp(TIMES, exprCopy(exp->left), exprDerivate(exprCopy(exp->right), var)));

        case DIV : 
            return exprOp(DIV, exprOp(MINUS, exprOp(TIMES, exprDerivate(exprCopy(exp->left), var), exprCopy(exp->right)), exprOp(TIMES, exprCopy(exp->left), exprDerivate(exprCopy(exp->right), var))), 
                                    exprOp(TIMES, exprCopy(exp->left), exprCopy(exp->left));

    }

    return NULL;
}
