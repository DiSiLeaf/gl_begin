#ifndef FUNCTION_ANNOTATION_MACRO_H
#define FUNCTION_ANNOTATION_MACRO_H

#ifdef VAR_IN
#error 'VAR_IN' macro defined
#endif
#ifdef VAR_OUT
#error 'VAR_OUT' macro defined
#endif
#ifdef VAR_INOUT
#error 'VAR_INOUT' macro defined
#endif

#define VAR_IN
#define VAR_OUT
#define VAR_INOUT


#endif