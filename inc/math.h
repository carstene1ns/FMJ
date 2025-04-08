#ifndef __MATH_H
#define __MATH_H

#pragma aux SIN "*"
extern short SIN[];

#pragma aux MakeSqrTable "*"
extern void MakeSqrTable(void);
extern unsigned int FindSqrt(unsigned int val);
#pragma aux FindTheta1 "*"
extern unsigned int FindTheta1(short ang);
#pragma aux FindTheta2 "*"
extern unsigned int FindTheta2(short ang);

#endif  // __MATH_H
