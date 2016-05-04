#include "Common.h"

int main(int argc, char* argv[])
{
	/*if (argc != 2)
	{
		printf("input directory error!\n");
		printf("%d", argc);
		exit(0);
	}*/
	//char* a = "C:\\Intel";				//test
	//DirWalk(a, TRUE);
	//DirWalk((char*)argv[1], TRUE);
	if (argc != 3)
	{
		printf("input error!\n");
		exit(0);
	}

	char src[100], des[100];

	//strcpy_s(src, argv[1]);
	//if (strlen(src) != 1 && src[strlen(src) - 1] == '\\')	//格式化输入
	//{
	//	src[strlen(src) - 1] = '\0';
	//}

	//int i;
	//for (i = strlen(src); i >= 0; i--)
	//{
	//	if (src[i] == '\\')
	//	{
	//		break;
	//	}
	//}

	//strcpy_s(des, argv[2]);
	//
	//if (strcmp(&src[i + 1], "") && strcmp(&src[i + 1], "..") && strcmp(&src[i + 1], "."))
	//{
	//	if (des[strlen(des) - 1] != '\\')
	//	{
	//		strcat_s(des, "/");
	//	}
	//	strcat_s(des, &src[i + 1]);

	//	SECURITY_ATTRIBUTES attribute;
	//	attribute.nLength = sizeof(attribute);
	//	attribute.lpSecurityDescriptor = NULL;
	//	attribute.bInheritHandle = FALSE;
	//	SetCurrentDirectory(argv[2]);

	//	CreateDirectory(&src[i + 1], &attribute);
	//}
	
	SECURITY_ATTRIBUTES attribute;
	attribute.nLength = sizeof(attribute);
	attribute.lpSecurityDescriptor = NULL;
	attribute.bInheritHandle = FALSE;
	CreateDirectory(argv[2], &attribute);

	DirWalk((char*)argv[1], (char*)argv[2], TRUE);			//ERROR?



	system("pause");
	return 0;
}

