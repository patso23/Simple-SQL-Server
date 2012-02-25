#include "node.h"

node::node(int type)
{
	for(int i=0;i<12;i++)
	{
		info[i]=-1;
	}
	info[1] = type;
	if (type == 4)
	{
		info[0]=0;
		info[4]=0;
	}
}

void node::setInfo(int num, int value)
{
	info[num]=value;
}

int node::getInfo(int num)
{
	return info[num];
}

int node::getType()
{
	return info[1];
}

void node::setType(int type)
{
	info[1] = type;
}

int node::getNumKey()
{
	int j=0;
	for(int i=4;i<=10;i=i+2)
	{
		if (info[i]!=-1)
		{
			j=j+1;
		}
	}
	return j;
}

void node::insert(node ptr1, int key, node ptr2)
{
	info[3] = ptr1.getInfo(0);
	info[4] = key;
	info[5] = ptr2.getInfo(0);
}

void node::insert(int key, int ptr)
{
	if (info[1]==3)
	{
		int i=4;
		while(info[i]<key&&info[i]!=-1)
		{
			i=i+2;
		}
		int tk;
		int tp;
		
		for(;i<=10;i=i+2)
		{
			tk=info[i];
			tp=info[i-1];
			info[i]=key;
			info[i-1]=ptr;
			key=tk;
			ptr=tp; 
		}
	}
	else
	{
		int i=4;
		while(info[i]<key&&info[i]!=-1)
		{
			i=i+2;
		}
		int tk;
		int tp;
		
		for(;i<=10;i=i+2)
		{
			tk=info[i];
			tp=info[i+1];
			info[i]=key;
			info[i+1]=ptr;
			key=tk;
			ptr=tp; 
		}
	}

}																			

bool node::search(int key)
{
	
	for(int i=4;i<10;i=i+2)
	{
		if(info[i] == key)
		{
			return true;
		}
	}
	return false;
}

int node::getPtr(int key)
{
	if (info[1]==3)
	{
		for(int i=4;i<=10;i=i+2)
		{
			if (info[i]==key)
				return info[i-1];
		}
	}
	else
		if(info[1]<3)
		{
			if (key >= info[10]&&info[10]!=-1)
			{
				return info[11];
			}
			else
			for (int j=4;j<=10;j=j+2)
			{
				if (key < info[j]&&info[j]!=-1)
				{
					return info[j-1];
				}
				else if (info[j]==-1)
				{
					return info[j-1];
				}
			}
		}
}

void node::copy(node temp)
{
	if (info[1]==3)
	{
		for (int i=3;i<9;i++)
		{
			info[i] = temp.getInfo(i-3);
		}
	}
	else
	{
		for (int j=3;j<8;j++)
		{
			info[j] = temp.getInfo(j-3);
		}
	}
}

void node::tail(node temp)
{
	if (info[1]==3)
	{
		for (int i=3;i<7;i++)
		{
			info[i] = temp.getInfo(i+3);
		}
	}
	else
	{
		for (int j=3;j<8;j++)
		{
			info[j] = temp.getInfo(j+3);
		}                       
	}
}

void node::clear()															//??????
{
	for (int i=3;i<12;i++)
	{
		info[i] = -1;
	}
}

int node::getKey(int num)
{
	return info[(num+1)*2];
}

void node::setNext(int ptr)
{
	info[11] = ptr;
}

int node::getNext()
{
	return info[11];
}

void node::addNum()//header
{
	info[4]=info[4]+1;
}

int node::getNum()//header get num of blocks
{
	return info[4];
}

void node::setRoot(node ptr)
{
	info[3] = ptr.getInfo(0);
}

int node::getRoot()
{
	return info[3];
}

int node::getNumPtr()
{
	int get=0;
	for(int i=2;i<11;i++)
	{
		if (info[i]!=-1)
		{
			get++;
		}
	}
	return get;
}

void node::addPtr(int ptr)
{
	for(int i=1;i<11;i++)
	{
		if(info[i]==-1)
		{
			info[i] = ptr;
			break;
		}
	}
}

void node::setBlockNum(int num)
{
	info[0] = num;
}

int node::getBlockNum()
{
	return info[0];
}

void node::setKeyAttr(int key)
{
	info[5] = key;
}

int node::getKeyAttr()
{
	return info[5];
}

void node::oversizeInsert(int key, int ptr)
{
	if (info[11]==3)
	{
		int i=1;
		while(info[i]<key&&info[i]!=-1)
		{
			i=i+2;
		}
		int tk;
		int tp;
		
		for(;i<=9;i=i+2)
		{
			tk=info[i];
			tp=info[i-1];
			info[i]=key;
			info[i-1]=ptr;
			key=tk;
			ptr=tp; 
		}
	}
	else
	{
		int i=1;
		while(info[i]<key&&info[i]!=-1)
		{
			i=i+2;
		}
		int tk;
		int tp;
		
		for(;i<=9;i=i+2)
		{
			tk=info[i];
			tp=info[i+1];
			info[i]=key;
			info[i+1]=ptr;
			key=tk;
			ptr=tp; 
		}
	}
}	
int node::oversizeGetKey(int num)
{
	return info[num*2-1];
}

void node::oversizeCopy(node nodeA)//
{	
	info[11] = nodeA.getInfo(1);
	if (info[11] == 3)
	{
		info[10] = nodeA.getInfo(11);
		for(int i=0;i<8;i++)
		{
			info[i] = nodeA.getInfo(i+3);
		}
	}
	else
	{
		for(int j=0;j<9;j++)
		{
			info[j] = nodeA.getInfo(j+3);
		}
	}	
}

void node::setDups(int dups)
{
	info[6]=dups;
}

int node::getDups()
{
	return info[6];
}
