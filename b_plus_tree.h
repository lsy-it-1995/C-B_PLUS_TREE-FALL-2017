#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H
#include <iostream>
#include <cassert>
#include <iomanip>
#include "arrayfunctions.h"
#include  <vector>

using namespace std;
template <class T>
class BPlusTree
{
public:
    class Iterator{
    public:
        friend class BPlusTree;
        Iterator(BPlusTree<T>* _it=NULL, int _key_ptr = 0):it(_it), key(_key_ptr){}

        T operator *()
        {
            bool debug = false;
            if(debug) cout<<"key: "<<key<<endl;
            if(debug) cout<<"data Count"<<it->data_count<<endl;
            assert(key<it->data_count);
            return it->data[key];
        }

        Iterator operator++(int un_used) //it++
        {
            Iterator hold = *this;
            if(this->key >= it->data_count - 1)
            {
                it = it->next;
                this->key = 0;
            }
            else
                ++this->key;
            return hold;
            //            Iterator add(it,key);
            //            if(key<it->data_count-1)
            //                key++;
            //            else
            //                it = it->next;
            //            return add;
        }
        Iterator operator++()   //++it
        {
            if(key == it->data_count-1)
            {
                it = it->next;
                key = 0;
            }
            else
                ++key;
            return *this;
        }
        bool at(const Iterator &other)
        {
            return it == other.it;
        }
        friend bool operator ==(const Iterator& lhs, const Iterator& rhs)
        {
            return (lhs.key == rhs.key && lhs.it == rhs.it);
        }

        friend bool operator !=(const Iterator& lhs, const Iterator& rhs)
        {
            return (lhs.key != rhs.key || lhs.it != rhs.it);
        }

        void print_Iterator()
        {
            print_array(it->data,it->data_count);
        }
        bool is_null()
        {
            return !it;
        }
    private:
        BPlusTree<T>* it;
        int key;
    };

    BPlusTree(bool dups = false){
        data_count = 0;
        child_count = 0;
        dups_ok = dups;
        next = NULL;
    }
    //big three:
    BPlusTree(const BPlusTree<T>& other)
    {
        data_count = 0;
        child_count = 0;
        next = NULL;
        copy_tree(other);
    }
    ~BPlusTree()
    {
        clear_tree();
    }
    BPlusTree<T>& operator =(const BPlusTree<T>& RHS)
    {
        if(this!=&RHS)
        {
            clear_tree();
            copy_tree(RHS);
        }
        return *this;
    }

    void insert(const T& entry);                //insert entry into the tree
    void remove(const T& entry);                //remove entry from the tree

    void clear_tree();                          //clear this object (delete all nodes etc.)
    void copy_tree(const BPlusTree<T>& other);      //copy other into this object
    void copy_tree(const BPlusTree<T>& other, vector<BPlusTree<T>* > &_stack)
    {
        bool debug = false;
        copy_array(data,other.data,other.data_count);
        data_count = other.data_count;
        child_count = other.child_count;
        if(debug) cout<<"data count"<<other.data_count<<endl;
        if(debug) cout<<"child count"<<other.child_count<<endl;
        for(int i = child_count-1;i>=0;--i)
        {
            if(debug) cout<<"why not"<<endl;
            subset[i] = new BPlusTree<T>(other.dups_ok);
            subset[i]->copy_tree(*other.subset[i],_stack);
            if(subset[i]->is_leaf() && (_stack.empty() && (i<= child_count -1)))
            {
                subset[i]->next = subset[i+1];
                if(i==0)
                    _stack.push_back(subset[i]);
            }
            else if(subset[i]->is_leaf() && (i == child_count -1))
            {
                if(_stack.size() != 0)
                {
                    subset[i]->next = _stack.front();
                    _stack.pop_back();
                }
            }

        }
    }
    bool contains(const T& entry);              //true if entry can be found in the array
    T& get(const T& entry);                     //return a reference to entry in the tree
    T& get_existing(const T& entry);            //return a reference to entry in the tree
    Iterator find(const T& entry)
    {
        //        BPlusTree<T> *temp = find_ptr(entry);
        //        return Iterator(temp);
        int i = first_ge(data, data_count, entry);
        if(!is_leaf())
        {
            if(data[i]==entry)
                return subset[i+1]->find(entry);
            else
                return subset[i]->find(entry);
        }
        else
        {
            if(data[i]==entry)
                return Iterator(this);
            else
                return Iterator(NULL);
        }
    }//return an iterator to this key. NULL if not there.

    int size() const
    {
        int number = 0;
        bool debug = false;
        if(debug) cout<<"in size"<<endl;
        if(debug) cout<<"datacount"<<this->data_count<<endl;
        if(debug) cout<<"childcount"<<this->child_count<<endl;
        int i = 0;
        if(!this->is_leaf())
        {
            if(debug) cout<<"in is NOT leaf"<<endl;
            while(this->child_count>i)
            {
                number += subset[i]->size();
                i++;
            }
        }
        else
        {
            if(debug) cout<<"in is leaf"<<endl;
            number += data_count;
        }
        return number;
    }//count the number of elements in the tree
    bool empty() const
    {
        return data_count == 0;
    }
    //true if the tree is empty

    void print_tree(int level = 0, ostream &outs=cout) const; //print a readable version of the tree
    friend ostream& operator<<(ostream& outs, const BPlusTree<T>& print_me)
    {
        print_me.print_tree(0, outs);
        return outs;
    }
    bool is_valid();
    Iterator begin();
    Iterator end();

private:
    static const int MINIMUM = 1;
    static const int MAXIMUM = 2 * MINIMUM;

    bool dups_ok;                       //true if duplicate keys may be inserted
    int data_count;                     //number of data elements
    T data[MAXIMUM + 1];                //holds the keys
    int child_count;                    //number of children
    BPlusTree* subset[MAXIMUM + 2];     //suBPlusTrees
    BPlusTree* next;
    bool is_leaf() const
    {
        return child_count==0;
    }        //true if this is a leaf node

    T* find_ptr(const T& entry);        //return a pointer to this key. NULL if not there.

    //insert element functions
    bool loose_insert(const T& entry);  //allows MAXIMUM+1 data elements in the root
    void fix_excess(int i);             //fix excess of data elements in child i

    //remove element functions:
    bool loose_remove(const T& entry);  //allows MINIMUM-1 data elements in the root
    void rotate_left(int i);
    void rotate_right(int i);
    void fix_shortage(int i);  //fix shortage of data elements in child i
    // and return the smallest key in this suBPlusTree
    BPlusTree<T> * get_smallest_node();
    BPlusTree<T> * get_biggest_node();
    void get_smallest(T& entry);        //entry := leftmost leaf
    void get_biggest(T& entry);         //entry := rightmost leaf
    void remove_biggest(T& entry);      //remove the biggest child of this tree->entry
    void transfer_left(int i);          //transfer one element LEFT from child i
    void transfer_right(int i);         //transfer one element RIGHT from child i
    void merge_with_next_subset(int i);    //merge subset i with subset i+1

};

//-----------------------------------------------------------------------------------------------
template<class T>
BPlusTree<T>* BPlusTree<T>::get_biggest_node()
{
    if(!subset[child_count-1]->is_leaf())
        return subset[child_count-1]->get_biggest_node();
    else
    {
        return subset[child_count-1];
    }
}
template<class T>
void BPlusTree<T>::get_biggest(T &entry)
{
    bool debug = false;
    if(!is_leaf())
    {
        subset[child_count-1]->get_biggest(entry);
    }
    else
    {
        entry = data[data_count-1];
    }
}

template<class T>
void BPlusTree<T>::rotate_left(int i)
{
    bool debug = false;
    T item;
    BPlusTree<T> *temp;

    if(!subset[i]->is_leaf())
    {
        if(debug) cout<<"FFFFFFFFFFFFFFFFFFFFFFFFFFFFF"<<endl;
        if(debug) cout<<"in !subset[i]->is_leaf()"<<endl;
        delete_item(subset[i+1]->subset,0,subset[i+1]->child_count,temp);
        attach_item(subset[i]->subset,subset[i]->child_count,temp);

    }
    if(debug) cout<<"rotate_leftrotate_leftrotate_leftrotate_leftrotate_left"<<endl;
    if(debug) cout<<"i: "<<i<<endl;
    if(debug) cout<<"is leaf or not"<<!subset[i]->is_leaf()<<endl;
    if(debug) cout<<"how many child in subset[i]"<<subset[i]->child_count<<endl;

    delete_item(subset[i+1]->data,0,subset[i+1]->data_count,item);
    ordered_insert(data,data_count,item);
    delete_item(data,i,data_count,item);
    ordered_insert(subset[i]->data,subset[i]->data_count,item);
    if(debug) cout<<"rotate_leftrotate_leftrotate_leftrotate_leftrotate_left"<<endl;
}
template<class T>
void BPlusTree<T>::rotate_right(int i)
{
    bool debug = false;
    T item;
    BPlusTree<T> *temp;

    if(!subset[i]->is_leaf())
    {
        if(debug) cout<<"OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"<<endl;
        if(debug) cout<<"in subset[i]->is_leaf()"<<endl;
        delete_item(subset[i-1]->subset,subset[i-1]->child_count-1,subset[i-1]->child_count,temp);
        insert_item(subset[i]->subset,0,subset[i]->child_count,temp);
    }

    if(debug) cout<<"rotate_rightrotate_rightrotate_rightrotate_rightrotate_rightrotate_right"<<endl;
    if(debug) cout<<"i: "<<i<<endl;
    if(debug) cout<<"is leaf or not"<<!subset[i]->is_leaf()<<endl;
    if(debug) cout<<"how many child in subset[i]"<<subset[i]->child_count<<endl;
    if(debug) cout<<"tree"<<endl;print_tree();
    delete_item(data,i-1,data_count,item);
    ordered_insert(subset[i]->data,subset[i]->data_count,item);
    detach_item(subset[i-1]->data,subset[i-1]->data_count,item);
    if(debug) cout<<"delete item: "<<item<<endl;
    ordered_insert(data,data_count,item);
    if(debug) cout<<"ordered insert item: "<<item<<endl;

    if(debug) cout<<"delete item: "<<item<<endl;
    if(debug) cout<<"tree"<<endl;print_tree();
    if(debug) cout<<"ordered inserted item: "<<item<<endl;
    if(debug) cout<<"rotate_rightrotate_rightrotate_rightrotate_rightrotate_rightrotate_right"<<endl;
}


template<class T>
BPlusTree<T>* BPlusTree<T>::get_smallest_node()
{
    if(!subset[0]->is_leaf())
        return subset[0]->get_smallest_node();
    else
    {
        return subset[0];
    }
}

template<class T>
void BPlusTree<T>::remove_biggest(T& entry)
{
    bool debug = false;
    if(debug) cout<<"remove biggest starts"<<endl;
    if(is_leaf())
    {
        if(debug) cout<<"isleaf"<<endl;
        detach_item(data,data_count,entry);
    }
    else
    {
        if(debug) cout<<"else"<<endl;
        subset[child_count-1]->remove_biggest(entry);
        if(debug) cout<<"subset[child_count-1]->data_count"<<subset[child_count-1]->data_count<<endl;
        if(subset[child_count-1]->data_count<MINIMUM)
        {
            if(debug) cout<<"remove biggest fix shortage"<<endl;
            fix_shortage(child_count-1);
        }
    }

}
template<class T>
T* BPlusTree<T>::find_ptr(const T& entry)
{
    int i = first_ge(data, data_count, entry);

    if(!is_leaf())
    {
        if(data[i]==entry)
            return subset[i+1]->find_ptr(entry);
        else
            return subset[i]->find_ptr(entry);
    }
    else
    {
        if(data[i]==entry)
            return &data[i];
        else
            return NULL;
    }
}
template<class T>
void BPlusTree<T>::get_smallest(T &entry)
{
    bool debug = false;
    if(debug) cout<<"get smallest"<<endl;
    if(debug) cout<<"entry: "<<entry<<endl;
    if(!is_leaf())
    {
        if(debug) cout<<"recurssion"<<endl;
        subset[0]->get_smallest(entry);
    }
    else
    {
        if(debug) cout<<"else in get smallest"<<endl;
        if(debug) cout<<"data[0]"<<data[0]<<endl;
        entry = data[0];
    }//replace entry with the left_most leaf of this tree
    // Keep looking in the first subtree (recursive)
    //  until you get to a leaf.
    // Then, replace entry with data[0] of the leaf node

}
template<class T>
bool BPlusTree<T>::contains(const T &entry)
{
    bool debug = false;
    if(debug) cout<<"-----------------------"<<endl;
    if(debug) cout<<"entry: "<<entry<<endl;
    return find_ptr(entry);
    //    int i = first_ge(data,data_count,entry);
    //    bool found = (i<data_count && data[i]==entry);

    //    if(debug) cout<<"data[i]: "<<data[i]<<endl;
    //    if(debug) cout<<"i: "<<i<<endl;
    //    if(found&&is_leaf())
    //    {
    //        if(debug) cout<<"FOUND"<<endl;
    //        if(debug) cout<<"data[i]"<<data[i]<<endl;
    //        if(debug) cout<<"i: "<<i<<endl;
    //        data[i] = entry;
    //        return true;
    //    }
    //    else if(found&&!is_leaf())
    //    {
    //        if(debug) cout<<"recurrsion"<<endl;
    //        return subset[i+1]->contains(entry);
    //    }
    //    else if(!found&&!is_leaf())
    //    {
    //        return subset[i]->contains(entry);
    //    }
    //    else
    //    {
    //        if(debug) cout<<"NOT FOUND"<<endl;
    //        return false;
    //    }
    if(debug) cout<<"-----------------------"<<endl;

}


template<class T>
typename BPlusTree<T>::Iterator BPlusTree<T>::end()
{
    return Iterator(NULL);
}

template<class T>
typename BPlusTree<T>::Iterator BPlusTree<T>::begin()
{
    return Iterator(get_smallest_node());
}

template<class T>
void BPlusTree<T>::merge_with_next_subset(int i)
{
    bool debug = false;
    T item;
    BPlusTree<T> *trash;
    if(debug) cout<<"merge with next subset"<<endl;
    if(debug) cout<<"i: "<<i<<endl;
    if(subset[i]->is_leaf())
    {
        if(debug) cout<<"is leaf"<<endl;
        delete_item(data,i,data_count,item);
        if(debug) cout<<"item: "<<item<<endl;
        merge(subset[i]->data,subset[i]->data_count,subset[i+1]->data,subset[i+1]->data_count);

        if(debug) cout<<"subset i array"<<endl;
        if(debug) print_array(subset[i]->data,subset[i]->data_count);
        if(debug) cout<<"after merge"<<endl;
        delete_item(subset,i+1,child_count,trash);
        subset[i]->next = trash->next;
        //        trash->child_count = 0;
        delete trash;
        //        subset[i]->next = subset[i+1];
        if(debug) cout<<"subset child count "<<subset[i]->data_count<<endl;

        if(debug) cout<<"end of merge if leaf"<<endl;
    }
    else
    {
        if(debug) cout<<"NOT is leaf"<<endl;
        if(debug) cout<<"i: "<<i<<endl;
        delete_item(data,i,data_count,item);
        if(debug) cout<<"item: "<<item<<endl;
        if(debug) cout<<"subset[i+1]->data[0]: "<<subset[i+1]->data[0]<<endl;
        attach_item(subset[i]->data,subset[i]->data_count,item);
        merge(subset[i]->data,subset[i]->data_count,subset[i+1]->data,subset[i+1]->data_count);
        merge(subset[i]->subset,subset[i]->child_count,subset[i+1]->subset,subset[i+1]->child_count);

        print_array(subset[i]->data,subset[i]->data_count);
        delete_item(subset,i+1,child_count,trash);
        delete trash;
        if(debug) cout<<"subset child count "<<subset[i]->child_count<<endl;
        if(debug) cout<<"end NOT is leaf"<<endl;

    }
    if(debug) cout<<"merge with next subset"<<endl;

}
template<class T>
void BPlusTree<T>::transfer_right(int i)
{
    /*
 * transfer_right:
 * when in transfer right, subset[i+1] is being fixed
 * transfer subset[i]'s last data[ ] to the beginning of subset[i+1]
 * data[i] is updated with subset[i+1]->data[0]
 */
    bool debug = false;
    if(debug) cout<<"transfer right"<<endl;
    if(debug) cout<<"dataC"<<data_count<<endl<<"childcount"<<child_count<<endl;
    T item;
    if(debug) cout<<"i: "<<i<<endl;
    delete_item(subset[i-1]->data,subset[i-1]->data_count-1,subset[i-1]->data_count,item);
    if(debug) cout<<"delete item: "<<item<<endl;
    ordered_insert(subset[i]->data,subset[i]->data_count,item);
    if(debug) cout<<"insert item: "<<item<<endl;
    delete_item(data,i,data_count,item);
    if(debug) cout<<"delete item: "<<item<<endl;
    ordered_insert(data,data_count,subset[i]->data[0]);
    if(debug) cout<<"insert item: "<<item<<endl;
    if(debug) print_tree();
    if(debug) cout<<"end of transfer right"<<endl;
    subset[i]->next = subset[i+1];
}

template<class T>
void BPlusTree<T>::transfer_left(int i)
{
    bool debug = false;
    if(debug) cout<<"transfer left"<<endl;
    T item;
    if(debug) cout<<"i: "<<i<<endl;
    delete_item(subset[i+1]->data,0,subset[i+1]->data_count,item);
    if(debug) cout<<"delete item: "<<item<<endl;
    attach_item(subset[i]->data,subset[i]->data_count,item);
    if(debug) cout<<"insert item: "<<item<<endl;
    delete_item(data,i,data_count,item);
    if(debug) cout<<"delete item: "<<item<<endl;
    ordered_insert(data,data_count,subset[i+1]->data[0]);
    if(debug) cout<<"insert item: "<<item<<endl;
    if(debug) print_tree();

    if(debug) cout<<"end of transfer left"<<endl;
    subset[i]->next = subset[i+1];
    /*
 * transfer_left
 * note: when in transfer left, subset[i-1] is being fixed
 * transfer subset[i]->data[0] to the end of subset[i-1]
 * data[i-1] is updated with subset[i]->data[0]
     */
}
template<class T>
void BPlusTree<T>:: fix_shortage(int i)
{
    bool debug = false;
    if(debug) cout<<"fix shortage"<<endl;
    if(debug) cout<<"i: "<<i<<endl;
    if(debug) cout<<"child count"<<child_count<<endl;
    if(debug) cout<<"data row"<<endl;
    if(debug) print_array(data,data_count);
    if(debug) cout<<"subset row i"<<endl;
    if(debug) print_array(subset[i]->data,subset[i]->data_count);
    if(debug) cout<<"subset row i+1"<<endl;

    if(subset[i]->is_leaf())
    {

        if(debug) cout<<"leaf in subset"<<endl;
        if(i<child_count-1&&subset[i+1]->data_count>MINIMUM)
        {
            if(debug) cout<<"i<child_count-1&&subset[i+1]->child_count>MINIMUM"<<endl;
            transfer_left(i);
        }
        else if(i>0&&subset[i-1]->data_count>MINIMUM)
        {
            if(debug) cout<<"i>0&&subset[i-1]->child_count>MINIMUM"<<endl;
            transfer_right(i);
        }
        else if(i<child_count-1)
        {
            if(debug) cout<<"i<child_count-1"<<endl;
            merge_with_next_subset(i); //merge left
        }
        else //i>0
        {
            if(debug) cout<<"i>0"<<endl;
            merge_with_next_subset(i-1); //merge right
        }
    }
    else
    {
        if(debug) cout<<"not leaf in subset"<<endl;
        if(i<child_count-1&&subset[i+1]->data_count>MINIMUM)
        {
            if(debug) cout<<"i<child_count-1&&subset[i+1]->child_count>MINIMUM"<<endl;
            rotate_left(i);
        }
        else if(i>0&&subset[i-1]->data_count>MINIMUM)
        {
            if(debug) cout<<"i>0&&subset[i-1]->child_count>MINIMUM"<<endl;
            rotate_right(i);
        }
        else if(i<child_count-1)
        {
            if(debug) cout<<"i<child_count-1"<<endl;
            merge_with_next_subset(i); //merge left
        }
        else //i>0
        {
            if(debug) cout<<"i>0"<<endl;
            merge_with_next_subset(i-1); //merge right
        }
    }
    if(debug) cout<<"mother fker"<<endl;
}

template<class T>
void BPlusTree<T>::remove(const T &entry)
{
    bool debug = false;
    if(loose_remove(entry))
    {
        if(debug) cout<<"REMOVE!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
        if(debug) cout<<"child count"<<child_count<<endl;
        if(debug) cout<<"data count"<<data_count<<endl;
        if((data_count == 0)&& (child_count ==1))
        {
            if(debug) cout<<"(data_count == 0) && (child_count ==1) "<<endl;
            BPlusTree<T> *temp = subset[0];
            copy_array(data,temp->data,temp->data_count);
            copy_array(subset,temp->subset,temp->child_count);
            data_count = temp->data_count;
            child_count = temp->child_count;
            temp->child_count = 0;
            delete temp;
        }
        if(debug) cout<<"REMVOE!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
    }
    else
    {
        cout<<"DIDN'T REMOVE THE ITEM(CAN'T FIND)"<<endl;
    }
    /*
     * ---------------------------------------------------------------------------------
     * Same as BTree:
     * Loose_remove the entry from this tree.
     * Shrink if you have to
     * ---------------------------------------------------------------------------------
     * once you return from loose_remove, the root (this object) may have no data and
     * only a single subset:
     * now, the tree must shrink:
     *
     * point a temporary pointer (shrink_ptr) and point it to this root's only subset
     * copy all the data and subsets of this subset into the root (through shrink_ptr)
     * now, the root contains all the data and poiners of it's old child.
     * now, simply delete shrink_ptr, and the tree has shrunk by one level.
     * Note, the root node of the tree will always be the same, it's the
     * child node we delete
     *
     *
     */
    /*
* remove(entry):
 *  call loose_remove(entry)
 *  shrink the tree if you need to.
 *
 *
 *                     [20|25]
 *                /       |      \
 *           [0|10] ->  [20] -> [25|30] ->|||
 *
     */
}
template<class T>
bool BPlusTree<T>::loose_remove(const T& entry)
{
    bool debug = false;
    int i = first_ge(data,data_count,entry);
    if(debug) cout<<"LOOSE REMOVE I: "<<i<<endl;
    bool found = i<data_count && data[i]==entry;
    T item = entry;
    if(found&&is_leaf())
    {
        if(debug) cout<<"found and is leaf"<<endl;
        delete_item(data,i,data_count,item);
        return true;
    }
    else if(!found&&!is_leaf())
    {
        if(debug) cout<<"i: "<<i<<endl;

        if(debug) cout<<"NOT found and NOT is leaf"<<endl;
        subset[i]->loose_remove(entry);
        if(subset[i]->data_count<MINIMUM)
        {
            if(debug) cout<<"not found not leaf special case"<<endl;
            if(debug) cout<<"i: "<<i<<endl;
            fix_shortage(i);
        }
        return true;
    }
    else if(!found && is_leaf())
    {
        if(debug) cout<<"NOT found and is leaf"<<endl;
        return false;
    }
    else // !is_leaf but found
    {
        if(debug) cout<<"found and NOT is leaf"<<endl;
        subset[i+1]->loose_remove(entry);
        if(subset[i+1]->data_count<MINIMUM)
        {

            if(debug) cout<<"if statement in fourth case"<<endl;
            if(debug) cout<<"i: "<<i<<endl;
            fix_shortage(i+1);
            if(debug) cout<<"end of if statement in fourth case"<<endl;
            if(debug) print_tree();
        }
        bool debug2 = false;
        i = first_ge(data,data_count,entry);
        found = i<data_count&& data[i]==entry;
        if(found)
        {
            if(debug2) cout<<"special if"<<endl;
            if(debug2) cout<<"found"<<endl;
            if(debug2) cout<<"i: "<<i<<endl;
            delete_item(data,i,data_count,item);
            if(debug2) cout<<"delete item: "<<item<<endl;
            if(debug2) cout<<"data count: "<<data_count<<endl;
            if(debug2) cout<<"child count: "<<child_count<<endl;
            subset[i+1]->get_smallest(item);
            if(debug2) cout<<"smallest item: "<<item<<endl;
            ordered_insert(data,data_count,item);
            if(debug2) cout<<"order number"<<item<<endl;
        }
        else
        {
            if(debug2) cout<<"not found SPECIAL CASE"<<endl;
            int j = first_ge(subset[i]->data,subset[i]->data_count,entry);
            if(debug2) cout<<"i: "<<i<<endl;
            if(debug2) cout<<"j: "<<j<<endl;
            bool found2 = j<subset[i]->data_count && subset[i]->data[j]==entry;
            if(debug2) cout<<"subset[i].data[0]"<<subset[i]->data[0]<<endl;
            if(debug2) cout<<"subset[i].data[1]"<<subset[i]->data[1]<<endl;
            if(found2)
            {
                if(debug2) cout<<"SSSSSSSSSSSSSSSSSSPIECAL"<<endl;
                if(debug2) cout<<"i: "<<i<<endl;
                if(debug2) cout<<"j: "<<j<<endl;
                delete_item(subset[i]->data,j,subset[i]->data_count,item);
                if(debug2) cout<<"delete item: "<<item<<endl;
                subset[i]->subset[j+1]->get_smallest(item);
                if(debug2) cout<<"get smallest item: "<<item<<endl;
                ordered_insert(subset[i]->data,subset[i]->data_count,item);
                if(debug2) cout<<"ordered insert item: "<<item<<endl;
            }
        }
        return true;
    }
    if(debug) cout<<"end of loose remove"<<endl;
}
template<class T>
bool BPlusTree<T>::is_valid()
{
    bool debug = false;
    if(!is_leaf())
    {
        int i = 0;
        if(debug) cout<<"inside the  NOT is leaf case"<<endl;
        subset[i++]->is_valid();
        if(debug) cout<<"child count "<<child_count<<endl;
        while(child_count>i)
        {
            if(debug) cout<<"int i: "<<i <<endl;
            subset[i++]->is_valid();
            if(debug) cout<<"almost there"<<endl;
        }
        if(data_count>1)
        {
            if(debug) cout<<"not is leaf case but datacount >1"<<endl;
            int check = 0;
            int follower = 0;
            while (follower<data_count-1)
            {
                if(debug) cout<<"second follower "<<follower<<endl;
                if(debug) cout<<"data["<<follower<<"]: "<<data[follower]<<endl;
                if(data[follower]<data[++follower])
                {
                    if(debug) cout<<"second follower "<<follower<<endl;
                    if(debug) cout<<"data["<<follower<<"]: "<<data[follower]<<endl;
                    check ++;
                    if(debug) cout<<"check: "<<check<<endl;
                }
            }
            if(debug) cout<<"dataC: "<<data_count<<endl<<"check: "<<check<<endl;
            if(check+1 != data_count)
                return false;
            if(debug) cout<<"not is leaf case but datacount >1"<<endl;
        }
        return true;

    }
    else
    {
        if(debug) cout<<"inside the is leaf case"<<endl;
        if(debug) cout<<"data count "<<data_count<<endl;
        if(debug) cout<<"child_count: "<<child_count<<endl;
        if(data_count>1)
        {
            int check2 = 0;
            int follower2 = 0;
            while (follower2<data_count-1)
            {
                if(debug) cout<<"second follower "<<follower2<<endl;

                if(debug) cout<<"data["<<follower2<<"]: "<<data[follower2]<<endl;
                if(data[follower2]<data[++follower2])
                {
                    if(debug) cout<<"second follower "<<follower2<<endl;
                    if(debug) cout<<"data["<<follower2<<"]: "<<data[follower2]<<endl;
                    check2 ++;
                    if(debug) cout<<"check: "<<check2<<endl;
                }
            }
            if(debug) cout<<"dataC: "<<data_count<<endl<<"check: "<<check2<<endl;
            if(check2+1 != data_count)
                return false;
        }
        if(debug) cout<<"inside the is leaf case"<<endl;
    }
    if(debug) cout<<"end of the case"<<endl;
}

template<class T>
void BPlusTree<T>::clear_tree()
{
    for(int i = 0; i< child_count;i++)
    {
        subset[i]->clear_tree();
        subset[i]->child_count = 0;
        data_count = 0;
        delete subset[i];
    }
    child_count = 0;
    data_count = 0;
}
template<class T>
void BPlusTree<T>::copy_tree(const BPlusTree<T>& other)
{
    vector< BPlusTree<T> *> stack;
    copy_tree(other, stack);
}
template<class T>
void BPlusTree<T>::insert(const T& entry)
{
    bool debug = false;
    if(loose_insert(entry))
    {
        if(data_count>MAXIMUM)
        {
            if(debug) cout<<"in insert datacount > max"<<endl;
            BPlusTree<T> *temp = new BPlusTree<T>(dups_ok);
            copy_array(temp->data,data,data_count);
            copy_array(temp->subset,subset,child_count);
            temp->child_count = child_count;
            temp->data_count = data_count;
            child_count = 1;
            data_count = 0;
            subset[0] = temp;
            fix_excess(0);
        }
    }
}//insert entry into the tree

template <class T>
T& BPlusTree<T>::get_existing(const T& entry)
{
    //    const bool debug = false;
    //       int i = first_ge(data, data_count, entry);
    //       bool found = (i<data_count && data[i]==entry);
    //       if (is_leaf())
    //       {
    //           if (found){
    //               return data[i];
    //           }
    //           else
    //           {
    //               if (debug) cout<<"get_existing was called with nonexistent entry"<<endl;
    //               assert(found);
    //           }
    //       }
    //       if (found) //inner node
    //           return subset[i+1]->get(entry);
    //           //or just return true?
    //       else //not found yet...
    //           return subset[i]->get(entry);


    bool debug = false;
    int i = first_ge(data, data_count, entry);
    bool found = (i<data_count && data[i]==entry);
    if(debug) cout<<"begin of get "<<endl;

    if(debug) cout<<"i: "<<i<<endl;
    if(debug) cout<<"entry: " << entry<<endl;

    if(found&&is_leaf())
    {
        if(debug) cout<<"found and leaf"<<endl;
        return data[i];
    }
    if(found&&!is_leaf())
    {
        if(debug) cout<<"found and not leaf"<<endl;
        return subset[i+1]->get(entry);
    }
    if(!found &&!is_leaf())
    {
        if(debug) cout<<"not found and not leaf"<<endl;
        return subset[i]->get(entry);
    }

}

template <class T>
T& BPlusTree<T>::get(const T &entry)
{
    bool debug = false;
    if(debug) cout<<"in get"<<endl;
    if(debug)
    {
        if(contains(entry))
            cout<<"true"<<endl;
        else
            cout<<"false"<<endl;
    }
    if(debug) cout<<"entry: "<<entry<<endl;
    if (!contains(entry))
    {
        if(debug) cout<<"insert in get not found 00000000000000000000000000000000000000000000000000000000000000000000"<<endl;
        insert(entry);
        //        return get(entry);
    }
    if(debug) cout<<"return get existing entry"<<endl;
    return get_existing(entry);

    //    bool debug = false;
    //    int i = first_ge(data, data_count, entry);
    //    bool found = (i<data_count && data[i]==entry);
    //    if(debug) cout<<"begin of get "<<endl;

    //    if(debug) cout<<"i: "<<i<<endl;
    //    if(debug) cout<<"entry: " << entry<<endl;

    //    if(found&&is_leaf())
    //    {
    //        if(debug) cout<<"found and leaf"<<endl;
    //        return data[i];
    //    }

    //    if(!found && is_leaf())
    //    {
    //        if(debug) cout<<"not found and is leaf"<<endl;
    //        insert(entry);
    //        return get(entry);
    //    }
    //    if(found&&!is_leaf())
    //    {
    //        if(debug) cout<<"found and not leaf"<<endl;
    //        subset[i+1]->get(entry);
    //        if(subset[i+1]->data_count>MAXIMUM)
    //        {
    //            if(debug) cout<<"subset[i+1]->data_count>MAXIMUM"<<endl;

    //            fix_excess(i+1);
    //        }
    //    }
    //    if(!found &&!is_leaf())
    //    {
    //        if(debug) cout<<"not found and not leaf"<<endl;

    //        subset[i]->get(entry);
    //        if(subset[i]->data_count>MAXIMUM)
    //        {
    //            if(debug) cout<<"subset[i]->data_count>MAXIMUM"<<endl;
    //            fix_excess(i);
    //        }
    //    }



    //    if(debug) cout<<"end of get "<<endl;
}
template<class T>
bool BPlusTree<T>::loose_insert(const T& entry)
{
    int i = first_ge(data, data_count, entry);
    bool found = i<data_count&&entry==data[i];
    bool debug = false;

    if(found&&dups_ok == false)
    {
        data[i] += entry;
        if(debug) cout<<"found and dups_ok"<<endl;
        return false;
    }

    if(found&&is_leaf())
    {
        if(debug) cout<<"found and isleaf"<<endl;
        data[i] = entry;

    }
    else if(found && !is_leaf())
    {
        if(debug) cout<<"found and !isleaf"<<endl;

        subset[i+1]->loose_insert(entry);
        if(subset[i+1]->data_count>MAXIMUM)
        {
            if(debug) cout<<"subset[i+1]->data_count>MAXIMUM"<<endl;
            fix_excess(i+1);
        }
        return true;
    }
    else if(!found&& is_leaf())
    {
        if(debug) cout<<"!found and isleaf"<<endl;

        ordered_insert(data,data_count,entry);
        return true;
    }
    else
    {
        if(debug) cout<<"!found and !isleaf"<<endl;
        subset[i]->loose_insert(entry);
        if(subset[i]->data_count>MAXIMUM)
        {
            if(debug) cout<<"subset[i]->data_count>MAXIMUM"<<endl;
            fix_excess(i);
        }
        return true;
    }
    /*
     loose_insert(entry):
     look for entry in data [ ]
     if found and no dupes, exit.
     four cases:
     1. found / leaf: call overloaded +
     2. found / !leaf call subset[i+1]->loose_insert and fix_excess(i+1)
     3. !found / leaf insert entry at position data[i]
     4. !found / !leaf call subset[i]->loose_insert and fix_excess(i)
     allows MAXIMUM+1 data elements in the root
     */
}

template<class T>
void BPlusTree<T>::fix_excess(int i)
{
    bool debug = false;
    BPlusTree<T> *temp = new BPlusTree<T>(dups_ok);
    T item;

    if(debug) cout<<"i: "<<i<<endl;
    if(debug) cout<<"data count: "<<data_count<<endl;
    if(subset[i]->is_leaf())
    {
        insert_item(subset,i+1,child_count,temp);
        split(subset[i]->data,subset[i]->data_count,subset[i+1]->data,subset[i+1]->data_count);
        detach_item(subset[i]->data,subset[i]->data_count,item);
        ordered_insert(subset[i+1]->data,subset[i+1]->data_count,item);
        ordered_insert(data,data_count,item);
        subset[i+1]->next = subset[i]->next;

        subset[i]->next = subset[i+1];
    }
    else
    {
        insert_item(subset,i+1,child_count,temp);
        split(subset[i]->data,subset[i]->data_count,subset[i+1]->data,subset[i+1]->data_count);
        split(subset[i]->subset,subset[i]->child_count,subset[i+1]->subset,subset[i+1]->child_count);
        detach_item(subset[i]->data,subset[i]->data_count,item);
        ordered_insert(data,data_count,item);
    }
    /*
fix_excess: when you split, insert middle item up to data[i].
                  [20]
                /      \
           [0|10] -> [20|30] ->|||

                   [20]
                 /      \                + 25
           [0|10] -> [20|25|30] ->|||
    insert 25: split [20, 25, 30]: COPY middle item up to data[i]
                     [20|25]
                /       |      \
           [0|10] -> [20|25] -> [30] ->|||
     is_leaf(): Detach the last item (25) from the left subset (subset[i]) and insert it
                into the left of subset[i+1]

                     [20|25]
                /       |      \
          [0|10] ->  [20] -> [25|30] ->|||

fix excess of data elements in child i
*/
}

template<class T>
void BPlusTree<T>::print_tree(int level, ostream &outs) const
{
    bool debug = false;
    if(debug) cout<<"in print tree"<<endl;
    if(child_count>MINIMUM)
    {
        if(debug) cout<<"childcount>MIN"<<endl;
        if(debug) cout<<"child count: "<<child_count<<endl;

        subset[child_count-1]->print_tree(level+1,outs);
    }
    if(debug) cout<<"data count: "<<data_count<<endl;
    outs<<setw(level*4)<<"==="<<endl;

    for(int i = 0; i <data_count;i++)
    {

        outs<<setw(level*4)<<data[data_count-i-1]<<endl;
        if(i+1==data_count)
        {
            outs<<setw(level*4)<<"==="<<endl;
        }
        if(!is_leaf())
        {
            if(debug) cout<<"!is_leaf"<<endl;
            subset[data_count-i-1]->print_tree(level+1,outs);
        }
    }


    if(debug) cout<<"end of print tree"<<endl;
}
#endif // B_PLUS_TREE_H
