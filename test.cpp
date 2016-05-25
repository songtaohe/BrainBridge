#include "test.h"


namespace android{






int A::function1()
{
	int i = 5;
	i = i + a;
	i = i+ this->a;
	i = i+ (*this).a;
	return i;
};

int A::function2()
{
	int i;
	i = function1();
	return i;
};

int A::function3()
{
	int i,j,k;
				


	i = function1();
	j = this->function2();
	k = 5 + b;


	return i+j+k;		
}



int A::function4()
{
	int i,j,k;
	A* mA = new A();

	mA->b = 0;	
	i = mA->function3();

	mA->mB.b = 10;






	return 100;
}







}
