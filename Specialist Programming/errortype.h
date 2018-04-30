// Errortype.h		Version 7		8/12/03
// Defines the values of certain integers used for error messages.

// Note that this list of error values may be increased to allow for error-specific types
// Do not check for failure with if(ErrorType = FAILURE). Use if(ErrorType!-=SUCCESS) to allow
// for future-proof code


#pragma once

enum ErrorType{SUCCESS, FAILURE};

