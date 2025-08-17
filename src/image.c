#include <wvcommon.h>

static char** path_list;

void QueueImage(const char* path, const char* title){
	int count = SendMessage(hListbox, LB_GETCOUNT, 0, 0);
	char* s = DuplicateString(path);
	SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)title);

	arrput(path_list, s);
	if(count == 0) ShowImage(0);
}

void ShowImage(int index){
	printf("%s\n", path_list[index]);
}

void DeleteImage(int index){
	SendMessage(hListbox, LB_DELETESTRING, index, 0);
	free(path_list[index]);
	arrdel(path_list, index);
}
