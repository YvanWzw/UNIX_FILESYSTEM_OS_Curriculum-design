#include "globalVariable.h"

using namespace std;

int main()
{
	turnOn(transformConstCharSToCharS("sb.dat"), false);
	int statusCode = 0;
	while (statusCode != SHUTDOWN)
	{
		statusCode = getOrder();
		printErrorInfo(statusCode);
		cout << endl;
	}
	shutDown();
	system("pause");
	return 0;
};