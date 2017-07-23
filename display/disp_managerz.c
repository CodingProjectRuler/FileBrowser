
#include <display_manager.h>
#include <string.h>

static PT_DispOpr g_ptDispOprHead;
PT_DispOpr g_ptDisOprShare = NULL;

int RegisterDispOpr(PT_DispOpr ptDispOpr)
{
	PT_DispOpr ptTemp;
	ptDispOpr->ptNext = NULL;
	if(g_ptDispOprHead == NULL)
		g_ptDispOprHead = ptDispOpr;
	else{
		ptTemp = g_ptDispOprHead;
		while(ptTemp->ptNext != NULL)
			ptTemp = ptTemp->ptNext;
		ptTemp->ptNext = ptDispOpr;	
	}
	return 0;
}

/*Return a respectively "PT_DispOpr" according to the name*/
PT_DispOpr GetDispOpr(char *pcName)
{
	PT_DispOpr ptTemp;
	ptTemp = g_ptDispOprHead;
	
	while(ptTemp != NULL){
		if(strcmp(ptTemp->name, pcName) == 0){
			g_ptDisOprShare = ptTemp;
			return ptTemp;
		}
		ptTemp = ptTemp->ptNext;
	}
	return NULL;
}


/*加入链表的操作而已*/
int DisplayRegister(void)
{
	return FBRegister();  /*return 0*/
}
