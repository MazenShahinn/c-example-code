/*******************************************************************************
*
* Program: Programming Trick to Make Arrays Assignable
*
* Description: An example of a 'programming trick' to make arrays assigning in 
* C by wrapping them in a struct as a member.
*
* YouTube Lesson: https://www.youtube.com/watch?v=IQUaOPAO-50
*
* Author: Kevin Browne @ https://portfoliocourses.com
*
*******************************************************************************/

#include <stdio.h>

// Structs can have arrays as members such as this 
typedef struct 
{
    char product_code[10];
} Product;

int main(void)
{
    // The below will cause an error in C because while static arrays can be 
    // initialized with '=' they are not assignable 
    /*

    // initializatoin of the array is allowed 
    char product_code_A[10] = "123456789";
    char product_code_B[10];

    // but assignment like this is not allowed as arrays are not assignable
    product_code_B = product_code_A;
    
    */

    // Create two structs, initialize the A struct's product_code member
    Product A = {"123456789"};
    Product B;

    // Structs in C ARE assignable and so this will copy the string in the A 
    // struct's product_code member to the B struct's product_code member
    B = A; 

    // Output the string to verify
    printf("%s\n", B.product_code);

    return 0;
}