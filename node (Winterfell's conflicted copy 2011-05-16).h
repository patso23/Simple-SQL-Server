#ifndef NODE_H
#define NODE_H


class node
{
private:
	int info[12];   //1:type 3,5,7,9,11:pointers 4,6,8,10:keys       for type:1 root   2 internal   3 leaf   4 header   5 pointerbucket
	                //for header: 3 root 4 numOfBlock 5 keyAttr          for pointerB: 0 bnum  1 type  2-10  p   11  next
public:
        node (int type);
	void setInfo(int num, int value);
	int getInfo(int num);
	int getType();
	void setType(int type);
	int getNumKey();
	void insert(int key, int ptr);
	void insert(node ptr1, int key, node ptr2);
	bool search(int key);
	void copy(node temp);
	void tail(node temp);
	
	int getPtr(int key);
	void clear();
	int getKey(int num);
	void setNext(int ptr);//leaf
	int getNext();//leaf
	void addNum(); //header
	int getNum();//header
	void setRoot(node ptr);//header
	int getRoot();//header
	int getNumPtr();//ptrbucket
	void addPtr(int ptr);//ptrbucket
	void setBlockNum(int num);
	int getBlockNum();
	void setKeyAttr(int key);
	int getKeyAttr();
	void oversizeInsert(int key, int ptr);
	void oversizeCopy(node nodeA);
	int oversizeGetKey(int num);
	void setDups(int dups);
	int getDups();
};


#endif
