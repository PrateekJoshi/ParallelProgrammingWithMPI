/*
 * integral_seq.c
 *
 *  Created on: 16-Nov-2020
 *      Author: prateek
 *      Pg - 77
 *      Desc : Calculate definite integral using trapezoidal rule.
 *      The function f(x) is hardwired.
 *      Input : a,b , n
 *      Output : Estimate of integral from a to b of f(x) using n trapezoids
 */
#include <stdio.h>

float f(float x);		// Function we are integrating

int main(int argc, char **argv)
{
	float integral;		// Store result in integral
	float a,b;			// Left and right endpoints
	int n;				// no of trapezoids
	float h;			// trapezoid base width
	float x;
	int i;

	printf("Enter a, b and n \n");
	scanf("%f %f %d", &a, &b, &n);

	h = (b-a)/n;
	integral = ( f(a) + f(b))/2.0;
	x = a;
	for( int i = 1 ; i <= n-1; i++ )
	{
		x += h;
		integral += f(x);
	}
	integral = integral * h;

	printf("With n = %d trapezoids, our estimates \n", n);
	printf("of the integral from %f to %f = %f \n", a, b, integral);

	return 0;
}


// f(x) = x^2
float f(float x)
{
	return (x*x);
}






