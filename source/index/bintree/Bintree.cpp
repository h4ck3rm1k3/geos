#include "../../headers/indexBintree.h"

/**
* Ensure that the BinTreeInterval for the inserted item has non-zero extents.
* Use the current minExtent to pad it, if necessary
*/
BinTreeInterval* Bintree::ensureExtent(BinTreeInterval *itemInterval,double minExtent){
	double min=itemInterval->getMin();
	double max=itemInterval->getMax();
	// has a non-zero extent
	if (min!=max) return itemInterval;
	// pad extent
	if (min==max) {
		min=min-minExtent/2.0;
		max=min+minExtent/2.0;
	}
	return new BinTreeInterval(min, max);
}



Bintree::Bintree() {
	minExtent=1.0;
	root=new Root();
}

int Bintree::depth(){
	if (root!=NULL) return root->depth();
	return 0;
}

int Bintree::size() {
	if (root!=NULL) return root->size();
	return 0;
}

/**
* Compute the total number of nodes in the tree
*
* @return the number of nodes in the tree
*/
int Bintree::nodeSize(){
	if (root!=NULL) return root->nodeSize();
	return 0;
}

void Bintree::insert(BinTreeInterval *itemInterval,void* item){
	collectStats(itemInterval);
	BinTreeInterval *insertInterval=ensureExtent(itemInterval,minExtent);
	//int oldSize=size();
	root->insert(insertInterval,item);
	/* DEBUG
	int newSize=size();
	System.out.println("BinTree: size="+newSize+"   node size="+nodeSize());
	if (newSize <= oldSize) {
	System.out.println("Lost item!");
	root.insert(insertInterval, item);
	System.out.println("reinsertion size="+size());
	}
	*/
}

vector<void*>* Bintree::iterator() {
	vector<void*>* foundItems=new vector<void*>();
	root->addAllItems(foundItems);
	return foundItems;
}

vector<void*>* Bintree::query(double x) {
	return query(new BinTreeInterval(x, x));
}

/**
* min and max may be the same value
*/
vector<void*>* Bintree::query(BinTreeInterval *interval) {
	/**
	* the items that are matched are all items in intervals
	* which overlap the query interval
	*/
	vector<void*>* foundItems=new vector<void*>();
	query(interval,foundItems);
	return foundItems;
}

void Bintree::query(BinTreeInterval *interval,vector<void*> *foundItems) {
	root->addAllItemsFromOverlapping(interval,foundItems);
}

void Bintree::collectStats(BinTreeInterval *interval) {
	double del=interval->getWidth();
	if (del<minExtent && del>0.0)
		minExtent=del;
}
