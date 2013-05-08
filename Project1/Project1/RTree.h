#ifndef RTREE_H
#define RTREE_H

// NOTE This file compiles under MSVC 6 SP5 and MSVC .Net 2003 it may not work on other compilers without modification.

// NOTE These next few lines may be win32 specific, you may need to modify them to compile on other platform
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include <limits> 
#include <opencv2\opencv.hpp>
#include<highgui.h>
using namespace cv;

#define ASSERT assert // RTree uses ASSERT( condition )
#ifndef Min
  #define Min __min 
#endif //Min
#ifndef Max
  #define Max __max 
#endif //Max

//
// RTree.h
//

#define RTREE_TEMPLATE template<class DATATYPE, class ELEMTYPE,int NUMDIMS, class ELEMTYPEREAL, int TMAXNODES, int TMINNODES>
#define RTREE_QUAL RTree<DATATYPE, ELEMTYPE,NUMDIMS, ELEMTYPEREAL, TMAXNODES, TMINNODES>

#define RTREE_DONT_USE_MEMPOOLS // This version does not contain a fixed memory allocator, fill in lines with EXAMPLE to implement one.
#define RTREE_USE_SPHERICAL_VOLUME // Better split classification, may be slower on some systems

// Fwd decl
class RTFileStream;  // File I/O helper class, look below for implementation and notes.


/// \class RTree
/// Implementation of RTree, a multidimensional bounding rectangle tree.
/// Example usage: For a 3-dimensional tree use RTree<Object*, float, 3> myTree;
///
/// This modified, templated C++ version by Greg Douglas at Auran (http://www.auran.com)
///
/// DATATYPE Referenced data, should be int, void*, obj* etc. no larger than sizeof<void*> and simple type
/// ELEMTYPE Type of element such as int or float
/// NUMDIMS Number of dimensions such as 2 or 3
/// ELEMTYPEREAL Type of element that allows fractional and large values such as float or double, for use in volume calcs
///
/// NOTES: Inserting and removing data requires the knowledge of its constant Minimal Bounding Rectangle.
///        This version uses new/delete for nodes, I recommend using a fixed size allocator for efficiency.
///        Instead of using a callback function for returned results, I recommend and efficient pre-sized, grow-only memory
///        array similar to MFC CArray or STL Vector for returning search query result.
///
template<class DATATYPE, class ELEMTYPE,  int NUMDIMS,
         class ELEMTYPEREAL = ELEMTYPE, int TMAXNODES = 8, int TMINNODES = TMAXNODES / 2>
class RTree
{
public: 

  struct Node;  // Fwd decl.  Used by other internal structs and iterator

public:
  
  //visit time
  int visitCount;
  // These constant must be declared after Branch and before Node struct
  // Stuck up here for MSVC 6 compiler.  NSVC .NET 2003 is much happier.
  enum
  {
    MAXNODES = TMAXNODES,                         ///< Max elements in node
    MINNODES = TMINNODES,                         ///< Min elements in node
  };


public:

  RTree();

  virtual ~RTree();
  
  /// Insert entry
  /// \param a_min Min of bounding rect
  /// \param a_max Max of bounding rect
  /// \param a_dataId Positive Id of data.  Maybe zero, but negative numbers not allowed.
  void Insert(const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId);
  void Insert(const ELEMTYPE a[NUMDIMS], const DATATYPE& a_dataId);
  
  /// Remove entry
  /// \param a_min Min of bounding rect
  /// \param a_max Max of bounding rect
  /// \param a_dataId Positive Id of data.  Maybe zero, but negative numbers not allowed.
  void Remove(const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId);



  /*----------------------R+ Remove-----------------------*/
  /// Remove entry
  /// \param a_min Min of bounding rect
  /// \param a_max Max of bounding rect
  /// \param a_dataId Positive Id of data.  Maybe zero, but negative numbers not allowed.
  void RemovePoint(const ELEMTYPE apoint[NUMDIMS], const DATATYPE& a_dataId);


  /*------------------------------------------------------*/


  /// Find all within search rectangle
  /// \param a_min Min of search bounding rect
  /// \param a_max Max of search bounding rect
  /// \param a_searchResult Search result array.  Caller should set grow size. Function will reset, not append to array.
  /// \param a_resultCallback Callback function to return result.  Callback should return 'true' to continue searching
  /// \param a_context User context to pass as parameter to a_resultCallback
  /// \return Returns the number of entries found
  int Search(const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], bool __cdecl a_resultCallback(DATATYPE a_data, void* a_context), void* a_context);
  int Search(const ELEMTYPE a_min[NUMDIMS], bool __cdecl a_resultCallback(DATATYPE a_data, void* a_context), void* a_context);
  
  /// Remove all entries from tree
  void RemoveAll();

  /// Count the data elements in this container.  This is slow as no internal counter is maintained.
  int Count();

  /// Load tree contents from file
  bool Load(const char* a_fileName);
  /// Load tree contents from stream
  bool Load(RTFileStream& a_stream);

  
  /// Save tree contents to file
  bool Save(const char* a_fileName);
  /// Save tree contents to stream
  bool Save(RTFileStream& a_stream);

  /// Iterator is not remove safe.
  class Iterator
  {
  private:
  
    enum { MAX_STACK = 32 }; //  Max stack size. Allows almost n^32 where n is number of branches in node
    
    struct StackElement
    {
      Node* m_node;
      int m_branchIndex;
    };
    
  public:
  
    Iterator()                                    { Init(); }

    ~Iterator()                                   { }
    
    /// Is iterator invalid
    bool IsNull()                                 { return (m_tos <= 0); }

    /// Is iterator pointing to valid data
    bool IsNotNull()                              { return (m_tos > 0); }

    /// Access the current data element. Caller must be sure iterator is not NULL first.
    DATATYPE& operator*()
    {
      ASSERT(IsNotNull());
      StackElement& curTos = m_stack[m_tos - 1];
      return curTos.m_node->m_branch[curTos.m_branchIndex].m_data;
    } 

    /// Access the current data element. Caller must be sure iterator is not NULL first.
    const DATATYPE& operator*() const
    {
      ASSERT(IsNotNull());
      StackElement& curTos = m_stack[m_tos - 1];
      return curTos.m_node->m_branch[curTos.m_branchIndex].m_data;
    } 

    /// Find the next data element
    bool operator++()                             { return FindNextData(); }

    /// Get the bounds for this node
    void GetBounds(ELEMTYPE a_min[NUMDIMS], ELEMTYPE a_max[NUMDIMS])
    {
      ASSERT(IsNotNull());
      StackElement& curTos = m_stack[m_tos - 1];
      Branch& curBranch = curTos.m_node->m_branch[curTos.m_branchIndex];
      
      for(int index = 0; index < NUMDIMS; ++index)
      {
        a_min[index] = curBranch.m_rect.m_min[index];
        a_max[index] = curBranch.m_rect.m_max[index];
      }
    }

  private:
  
    /// Reset iterator
    void Init()                                   { m_tos = 0; }

    /// Find the next data element in the tree (For internal use only)
    bool FindNextData()
    {
      for(;;)
      {
        if(m_tos <= 0)
        {
          return false;
        }
        StackElement curTos = Pop(); // Copy stack top cause it may change as we use it

        if(curTos.m_node->IsLeaf())
        {
          // Keep walking through data while we can
          if(curTos.m_branchIndex+1 < curTos.m_node->m_count)
          {
            // There is more data, just point to the next one
            Push(curTos.m_node, curTos.m_branchIndex + 1);
            return true;
          }
          // No more data, so it will fall back to previous level
        }
        else
        {
          if(curTos.m_branchIndex+1 < curTos.m_node->m_count)
          {
            // Push sibling on for future tree walk
            // This is the 'fall back' node when we finish with the current level
            Push(curTos.m_node, curTos.m_branchIndex + 1);
          }
          // Since cur node is not a leaf, push first of next level to get deeper into the tree
          Node* nextLevelnode = curTos.m_node->m_branch[curTos.m_branchIndex].m_child;
          Push(nextLevelnode, 0);
          
          // If we pushed on a new leaf, exit as the data is ready at TOS
          if(nextLevelnode->IsLeaf())
          {
            return true;
          }
        }
      }
    }

    /// Push node and branch onto iteration stack (For internal use only)
    void Push(Node* a_node, int a_branchIndex)
    {
      m_stack[m_tos].m_node = a_node;
      m_stack[m_tos].m_branchIndex = a_branchIndex;
      ++m_tos;
      ASSERT(m_tos <= MAX_STACK);
    }
    
    /// Pop element off iteration stack (For internal use only)
    StackElement& Pop()
    {
      ASSERT(m_tos > 0);
      --m_tos;
      return m_stack[m_tos];
    }

    StackElement m_stack[MAX_STACK];              ///< Stack as we are doing iteration instead of recursion
    int m_tos;                                    ///< Top Of Stack index
  
    friend RTree; // Allow hiding of non-public functions while allowing manipulation by logical owner
  };

  /// Get 'first' for iteration
  void GetFirst(Iterator& a_it)
  {
    a_it.Init();
    Node* first = m_root;
    while(first)
    {
      if(first->IsInternalNode() && first->m_count > 1)
      {
        a_it.Push(first, 1); // Descend sibling branch later
      }
      else if(first->IsLeaf())
      {
        if(first->m_count)
        {
          a_it.Push(first, 0);
        }
        break;
      }
      first = first->m_branch[0].m_child;
    }
  }  

  /// Get Next for iteration
  void GetNext(Iterator& a_it)                    { ++a_it; }

  /// Is iterator NULL, or at end?
  bool IsNull(Iterator& a_it)                     { return a_it.IsNull(); }

  /// Get object at iterator position
  DATATYPE& GetAt(Iterator& a_it)                 { return *a_it; }



public:

  /// Minimal bounding rectangle (n-dimensional)
  struct Rect
  {
    ELEMTYPE m_min[NUMDIMS];                      ///< Min dimensions of bounding box 
    ELEMTYPE m_max[NUMDIMS];                      ///< Max dimensions of bounding box 
  };

  // Data structure of inserted point
  struct Point
  {
	  ELEMTYPE m[NUMDIMS];
  };

  /// 用于求取knn的时候使用
  struct NeighbourNode
  {
	DATATYPE id;           //记录节点的id
	double distance;     //记录两者之间的距离
  };

  /// May be data or may be another subtree
  /// The parents level determines this.
  /// If the parents level is 0, then this is data
  struct Branch
  {
    Rect m_rect;                                  ///< Bounds
    union
    {
      Node* m_child;                              ///< Child node
      DATATYPE m_data;                            ///< Data Id or Ptr
    };
  };

  /// Node for each branch level
  struct Node
  {
    bool IsInternalNode()                         { return (m_level > 0); } // Not a leaf, but a internal node
    bool IsLeaf()                                 { return (m_level == 0); } // A leaf, contains data
    
    int m_count;                                  ///< Count
    int m_level;                                  ///< Leaf is zero, others positive
    Branch m_branch[MAXNODES];                    ///< Branch
  };
  
  /// A link list of nodes for reinsertion after a delete operation
  struct ListNode
  {
    ListNode* m_next;                             ///< Next in list
    Node* m_node;                                 ///< Node
  };

  struct BranchArray
  {
	  int m_count;
	  int m_sortIndex[MAXNODES+1];
	  double m_sortNumber[MAXNODES+1];
	  double m_assistSortNumber[MAXNODES+1];
	  Branch m_branchBuf[MAXNODES+1];
  };

  /// Variables for finding a split partition
  struct PartitionVars
  {
    int m_partition[MAXNODES+1];
    int m_total;
    int m_minFill;
    int m_taken[MAXNODES+1];
    int m_count[2];
    Rect m_cover[2];
    ELEMTYPEREAL m_area[2];

    Branch m_branchBuf[MAXNODES+1];
    int m_branchCount;
    Rect m_coverSplit;
    ELEMTYPEREAL m_coverSplitArea;
  }; 
 
  Node* AllocNode();
  void FreeNode(Node* a_node);
  void InitNode(Node* a_node);
  void InitRect(Rect* a_rect);
  bool InsertPointPot(Point* a_point, const DATATYPE& a_id, Node* a_node, Node** a_newNode, int a_level);
  bool InsertRectRec(Rect* a_rect, const DATATYPE& a_id, Node* a_node, Node** a_newNode, int a_level);
  bool InsertPoint(Point* a_point, const DATATYPE& a_id, Node** a_root, int a_level);
  bool InsertRect(Rect* a_rect, const DATATYPE& a_id, Node** a_root, int a_level);
  Rect NodeCover(Node* a_node);
  bool AddBranch(Branch* a_branch, Node* a_node, Node** a_newNode);
  bool NewAddBranch(Branch* a_branch, Node* a_node, Node** a_newNode);
  void DisconnectBranch(Node* a_node, int a_index);
  int PickBranch(Rect* a_rect, Node* a_node);
  int PickBranchSimple(Point* a_point, Node* a_node);
  int PickBranch(Point* a_point, Node* a_node);
  bool ReorganizeNode(Point* a_point, Node* a_node, Node** a_newNode, int* index);
  Rect CombineRect(Rect* a_rectA, Rect* a_rectB);
  Rect EnlargeRect(Point* a_point, Rect* a_rectB);
  void SplitNode(Node* a_node, Branch* a_branch, Node** a_newNode);
  ELEMTYPEREAL RectSphericalVolume(Rect* a_rect);
  ELEMTYPEREAL RectVolume(Rect* a_rect);
  ELEMTYPEREAL CalcRectVolume(Rect* a_rect);
  void GetBranches(Node* a_node, Branch* a_branch, PartitionVars* a_parVars);
  void ChoosePartition(PartitionVars* a_parVars, int a_minFill);
  void LoadNodes(Node* a_nodeA, Node* a_nodeB, PartitionVars* a_parVars);
  void InitParVars(PartitionVars* a_parVars, int a_maxRects, int a_minFill);
  void PickSeeds(PartitionVars* a_parVars);
  void Classify(int a_index, int a_group, PartitionVars* a_parVars);
  bool RemoveRect(Rect* a_rect, const DATATYPE& a_id, Node** a_root);
  bool RemoveRectRec(Rect* a_rect, const DATATYPE& a_id, Node* a_node, ListNode** a_listNode);
  ListNode* AllocListNode();
  void FreeListNode(ListNode* a_listNode);
  bool Overlap(Rect* a_rectA, Rect* a_rectB);
  void ReInsert(Node* a_node, ListNode** a_listNode);
  bool Search(Node* a_node, Rect* a_rect, int& a_foundCount, bool __cdecl a_resultCallback(DATATYPE a_data, void* a_context), void* a_context);
  void RemoveAllRec(Node* a_node);
  void Reset();
  void CountRec(Node* a_node, int& a_count);
  /*计算node的k近邻的函数，输入k，和结果数组的引用，结果类型的id列表*/
  ELEMTYPE minDist(Rect* a_rect, ELEMTYPE queryPoint[NUMDIMS]);
  void KNearNeighbour(ELEMTYPE queryPoint[NUMDIMS], Node* a_node, int k, NeighbourNode* knnList, double& ndistance);


  bool SaveRec(Node* a_node, RTFileStream& a_stream);
  bool LoadRec(Node* a_node, RTFileStream& a_stream);

  bool RemovePoint(Point* a_point, const DATATYPE& a_id, Node** a_root);
  bool RemovePointPoi(Point* a_point, const DATATYPE& a_id, Node* a_node, ListNode** a_listNode);
  bool OverlapPoint(Point* a_point, Rect* a_rectB);

  void NewSplitNode(Node* a_node, Branch* a_branch, Node** a_newNode);
  void GetBranches(Node* a_node, Branch* a_branch, BranchArray* branchArray);
  void ChoosePartition(BranchArray* branchArray, Node* a_node, Node* a_newNode);
  void Partition(BranchArray* branchArray, Node* a_node, Node* a_newNode, int patitionAxis, double border);
  void sortByAxis(BranchArray* branchArray, int partitionAxis);
  void sortAssistNumByAxis(BranchArray* branchArray, int partitionAxis);
  void ChoosePartitionValue(Point* a_point, Rect rectA, Rect rectB, double* bestBorder, int* bestPartition);
  void setNode(Node* a_node, Point* a_point);

  // need delete
  void drawNode(Node* a_node , Mat* image, int wid)
  {
	  char wndname[] = "Drawing Demo";
	  cv::Point p1, p2;
	  if (a_node->IsInternalNode())
	  {
		  for (int i = 0;i < a_node->m_count;i++)
		  {
			  p1.x = a_node->m_branch[i].m_rect.m_max[0]/7;
			  p1.y = a_node->m_branch[i].m_rect.m_max[1]/7;
			  p2.x = a_node->m_branch[i].m_rect.m_min[0]/7;
			  p2.y = a_node->m_branch[i].m_rect.m_min[1]/7;
			  rectangle( *image, p1, p2, Scalar(255, 255, 255), wid, CV_AA );
			  imshow(wndname, *image);
			  if(waitKey(5) >= 0) return;
			  if (wid > 2)
			  drawNode(a_node->m_branch[i].m_child, image, wid-2);
			  else
				  drawNode(a_node->m_branch[i].m_child, image, wid);
		  }
	  }
	  else
	  {
		  for (int i = 0;i < a_node->m_count;i++)
		  {
			  p1.x = a_node->m_branch[i].m_rect.m_max[0]/7;
			  p1.y = a_node->m_branch[i].m_rect.m_max[1]/7;
			  circle( *image, p1, 1, Scalar(255, 255, 255), 1, CV_AA );
		  }
	  }
  }
  
                                   ///< Root of tree
  ELEMTYPEREAL m_unitSphereVolume;                 ///< Unit sphere constant for required number of dimensions
  double scale;
  Node* m_root;   

  public:
		DATATYPE*  KNN(ELEMTYPE queryPoint[NUMDIMS], int k);
};


// Because there is not stream support, this is a quick and dirty file I/O helper.
// Users will likely replace its usage with a Stream implementation from their favorite API.
class RTFileStream
{
  FILE* m_file;

public:

  
  RTFileStream()
  {
    m_file = NULL;
  }

  ~RTFileStream()
  {
    Close();
  }

  bool OpenRead(const char* a_fileName)
  {
    m_file = fopen(a_fileName, "rb");
    if(!m_file)
    {
      return false;
    }
    return true;
  }

  bool OpenWrite(const char* a_fileName)
  {
    m_file = fopen(a_fileName, "wb");
    if(!m_file)
    {
      return false;
    }
    return true;
  }

  void Close()
  {
    if(m_file)
    {
      fclose(m_file);
      m_file = NULL;
    }
  }

  template< typename TYPE >
  size_t Write(const TYPE& a_value)
  {
    ASSERT(m_file);
    return fwrite((void*)&a_value, sizeof(a_value), 1, m_file);
  }

  template< typename TYPE >
  size_t WriteArray(const TYPE* a_array, int a_count)
  {
    ASSERT(m_file);
    return fwrite((void*)a_array, sizeof(TYPE) * a_count, 1, m_file);
  }

  template< typename TYPE >
  size_t Read(TYPE& a_value)
  {
    ASSERT(m_file);
    return fread((void*)&a_value, sizeof(a_value), 1, m_file);
  }

  template< typename TYPE >
  size_t ReadArray(TYPE* a_array, int a_count)
  {
    ASSERT(m_file);
    return fread((void*)a_array, sizeof(TYPE) * a_count, 1, m_file);
  }
};


RTREE_TEMPLATE
RTREE_QUAL::RTree()
{
  ASSERT(MAXNODES > MINNODES);
  ASSERT(MINNODES > 0);


  // We only support machine word size simple data type eg. integer index or object pointer.
  // Since we are storing as union with non data branch
  ASSERT(sizeof(DATATYPE) == sizeof(void*) || sizeof(DATATYPE) == sizeof(int));

  // Precomputed volumes of the unit spheres for the first few dimensions
  const float UNIT_SPHERE_VOLUMES[] = {
    0.000000f, 2.000000f, 3.141593f, // Dimension  0,1,2
    4.188790f, 4.934802f, 5.263789f, // Dimension  3,4,5
    5.167713f, 4.724766f, 4.058712f, // Dimension  6,7,8
    3.298509f, 2.550164f, 1.884104f, // Dimension  9,10,11
    1.335263f, 0.910629f, 0.599265f, // Dimension  12,13,14
    0.381443f, 0.235331f, 0.140981f, // Dimension  15,16,17
    0.082146f, 0.046622f, 0.025807f, // Dimension  18,19,20 
  };

  m_root = AllocNode();
  m_root->m_level = 0;
  scale = 1.0;
  m_unitSphereVolume = (ELEMTYPEREAL)UNIT_SPHERE_VOLUMES[NUMDIMS];

  visitCount = 0;
}


RTREE_TEMPLATE
RTREE_QUAL::~RTree()
{
  Reset(); // Free, or reset node memory
}


RTREE_TEMPLATE
void RTREE_QUAL::Insert(const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId)
{
#ifdef _DEBUG
  for(int index=0; index<NUMDIMS; ++index)
  {
    ASSERT(a_min[index] <= a_max[index]);
  }
#endif //_DEBUG

  Rect rect;
  
  for(int axis=0; axis<NUMDIMS; ++axis)
  {
    rect.m_min[axis] = a_min[axis];
    rect.m_max[axis] = a_max[axis];
  }
  
  InsertRect(&rect, a_dataId, &m_root, 0);
}

RTREE_TEMPLATE
void RTREE_QUAL::Insert(const ELEMTYPE a[NUMDIMS],  const DATATYPE& a_dataId)
{
  Point point;
  
  for(int axis=0; axis<NUMDIMS; ++axis)
  {
    point.m[axis] = a[axis];
  }
  
  InsertPoint(&point, a_dataId, &m_root, 0);
}


RTREE_TEMPLATE
void RTREE_QUAL::Remove(const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], const DATATYPE& a_dataId)
{
#ifdef _DEBUG
  for(int index=0; index<NUMDIMS; ++index)
  {
    ASSERT(a_min[index] <= a_max[index]);
  }
#endif //_DEBUG

  Rect rect;
  
  for(int axis=0; axis<NUMDIMS; ++axis)
  {
    rect.m_min[axis] = a_min[axis];
    rect.m_max[axis] = a_max[axis];
  }

  RemoveRect(&rect, a_dataId, &m_root);
}

RTREE_TEMPLATE
void RTREE_QUAL::RemovePoint(const ELEMTYPE apoint[NUMDIMS], const DATATYPE& a_dataId)
{
	Point po;
	for (int axis=0; axis<NUMDIMS; axis++)
	{
		po.m[axis] = apoint[axis];
	}

	RemovePoint(&po, a_dataId, &m_root);
}


RTREE_TEMPLATE
int RTREE_QUAL::Search(const ELEMTYPE a_min[NUMDIMS], const ELEMTYPE a_max[NUMDIMS], bool __cdecl a_resultCallback(DATATYPE a_data, void* a_context), void* a_context)
{
#ifdef _DEBUG
  for(int index=0; index<NUMDIMS; ++index)
  {
    ASSERT(a_min[index] <= a_max[index]);
  }
#endif //_DEBUG

  Rect rect;
  
  for(int axis=0; axis<NUMDIMS; ++axis)
  {
    rect.m_min[axis] = a_min[axis];
    rect.m_max[axis] = a_max[axis];
  }

  // NOTE: May want to return search result another way, perhaps returning the number of found elements here.

  int foundCount = 0;
  Search(m_root, &rect, foundCount, a_resultCallback, a_context);
  
  return foundCount;
}


RTREE_TEMPLATE
int RTREE_QUAL::Count()
{
  int count = 0;
  CountRec(m_root, count);
  
  return count;
}

RTREE_TEMPLATE
void RTREE_QUAL::CountRec(Node* a_node, int& a_count)
{
  if(a_node->IsInternalNode())  // not a leaf node
  {
    for(int index = 0; index < a_node->m_count; ++index)
    {
      CountRec(a_node->m_branch[index].m_child, a_count);
    }
  }
  else // A leaf node
  {
    a_count += a_node->m_count;
  }
}


RTREE_TEMPLATE
bool RTREE_QUAL::Load(const char* a_fileName)
{
  RemoveAll(); // Clear existing tree

  RTFileStream stream;
  if(!stream.OpenRead(a_fileName))
  {
    return false;
  }

  bool result = Load(stream);
  
  stream.Close();

  return result;
};


RTREE_TEMPLATE
bool RTREE_QUAL::Load(RTFileStream& a_stream)
{
  // Write some kind of header
  int _dataFileId = ('R'<<0)|('T'<<8)|('R'<<16)|('E'<<24);
  int _dataSize = sizeof(DATATYPE);
  int _dataNumDims = NUMDIMS;
  int _dataElemSize = sizeof(ELEMTYPE);
  int _dataElemRealSize = sizeof(ELEMTYPEREAL);
  int _dataMaxNodes = TMAXNODES;
  int _dataMinNodes = TMINNODES;

  int dataFileId = 0;
  int dataSize = 0;
  int dataNumDims = 0;
  int dataElemSize = 0;
  int dataElemRealSize = 0;
  int dataMaxNodes = 0;
  int dataMinNodes = 0;

  a_stream.Read(dataFileId);
  a_stream.Read(dataSize);
  a_stream.Read(dataNumDims);
  a_stream.Read(dataElemSize);
  a_stream.Read(dataElemRealSize);
  a_stream.Read(dataMaxNodes);
  a_stream.Read(dataMinNodes);

  bool result = false;

  // Test if header was valid and compatible
  if(    (dataFileId == _dataFileId) 
      && (dataSize == _dataSize) 
      && (dataNumDims == _dataNumDims) 
      && (dataElemSize == _dataElemSize) 
      && (dataElemRealSize == _dataElemRealSize) 
      && (dataMaxNodes == _dataMaxNodes) 
      && (dataMinNodes == _dataMinNodes) 
    )
  {
    // Recursively load tree
    result = LoadRec(m_root, a_stream);
  }

  return result;
}


RTREE_TEMPLATE
bool RTREE_QUAL::LoadRec(Node* a_node, RTFileStream& a_stream)
{
  a_stream.Read(a_node->m_level);
  a_stream.Read(a_node->m_count);

  if(a_node->IsInternalNode())  // not a leaf node
  {
    for(int index = 0; index < a_node->m_count; ++index)
    {
      Branch* curBranch = &a_node->m_branch[index];

      a_stream.ReadArray(curBranch->m_rect.m_min, NUMDIMS);
      a_stream.ReadArray(curBranch->m_rect.m_max, NUMDIMS);

      curBranch->m_child = AllocNode();
      LoadRec(curBranch->m_child, a_stream);
    }
  }
  else // A leaf node
  {
    for(int index = 0; index < a_node->m_count; ++index)
    {
      Branch* curBranch = &a_node->m_branch[index];

      a_stream.ReadArray(curBranch->m_rect.m_min, NUMDIMS);
      a_stream.ReadArray(curBranch->m_rect.m_max, NUMDIMS);

      a_stream.Read(curBranch->m_data);
    }
  }

  return true; // Should do more error checking on I/O operations
}


RTREE_TEMPLATE
bool RTREE_QUAL::Save(const char* a_fileName)
{
  RTFileStream stream;
  if(!stream.OpenWrite(a_fileName))
  {
    return false;
  }

  bool result = Save(stream);

  stream.Close();

  return result;
}


RTREE_TEMPLATE
bool RTREE_QUAL::Save(RTFileStream& a_stream)
{
  // Write some kind of header
  int dataFileId = ('R'<<0)|('T'<<8)|('R'<<16)|('E'<<24);
  int dataSize = sizeof(DATATYPE);
  int dataNumDims = NUMDIMS;
  int dataElemSize = sizeof(ELEMTYPE);
  int dataElemRealSize = sizeof(ELEMTYPEREAL);
  int dataMaxNodes = TMAXNODES;
  int dataMinNodes = TMINNODES;

  a_stream.Write(dataFileId);
  a_stream.Write(dataSize);
  a_stream.Write(dataNumDims);
  a_stream.Write(dataElemSize);
  a_stream.Write(dataElemRealSize);
  a_stream.Write(dataMaxNodes);
  a_stream.Write(dataMinNodes);

  // Recursively save tree
  bool result = SaveRec(m_root, a_stream);
  
  return result;
}


RTREE_TEMPLATE
bool RTREE_QUAL::SaveRec(Node* a_node, RTFileStream& a_stream)
{
  a_stream.Write(a_node->m_level);
  a_stream.Write(a_node->m_count);

  if(a_node->IsInternalNode())  // not a leaf node
  {
    for(int index = 0; index < a_node->m_count; ++index)
    {
      Branch* curBranch = &a_node->m_branch[index];

      a_stream.WriteArray(curBranch->m_rect.m_min, NUMDIMS);
      a_stream.WriteArray(curBranch->m_rect.m_max, NUMDIMS);

      SaveRec(curBranch->m_child, a_stream);
    }
  }
  else // A leaf node
  {
    for(int index = 0; index < a_node->m_count; ++index)
    {
      Branch* curBranch = &a_node->m_branch[index];

      a_stream.WriteArray(curBranch->m_rect.m_min, NUMDIMS);
      a_stream.WriteArray(curBranch->m_rect.m_max, NUMDIMS);

      a_stream.Write(curBranch->m_data);
    }
  }

  return true; // Should do more error checking on I/O operations
}


RTREE_TEMPLATE
void RTREE_QUAL::RemoveAll()
{
  // Delete all existing nodes
  Reset();

  m_root = AllocNode();
  m_root->m_level = 0;
}


RTREE_TEMPLATE
void RTREE_QUAL::Reset()
{
#ifdef RTREE_DONT_USE_MEMPOOLS
  // Delete all existing nodes
  RemoveAllRec(m_root);
#else // RTREE_DONT_USE_MEMPOOLS
  // Just reset memory pools.  We are not using complex types
  // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
}


RTREE_TEMPLATE
void RTREE_QUAL::RemoveAllRec(Node* a_node)
{
  ASSERT(a_node);
  ASSERT(a_node->m_level >= 0);

  if(a_node->IsInternalNode()) // This is an internal node in the tree
  {
    for(int index=0; index < a_node->m_count; ++index)
    {
      RemoveAllRec(a_node->m_branch[index].m_child);
    }
  }
  FreeNode(a_node); 
}


RTREE_TEMPLATE
typename RTREE_QUAL::Node* RTREE_QUAL::AllocNode()
{
  Node* newNode;
#ifdef RTREE_DONT_USE_MEMPOOLS
  newNode = new Node;
#else // RTREE_DONT_USE_MEMPOOLS
  // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
  InitNode(newNode);
  return newNode;
}


RTREE_TEMPLATE
void RTREE_QUAL::FreeNode(Node* a_node)
{
  ASSERT(a_node);

#ifdef RTREE_DONT_USE_MEMPOOLS
  delete a_node;
#else // RTREE_DONT_USE_MEMPOOLS
  // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
}


// Allocate space for a node in the list used in DeletRect to
// store Nodes that are too empty.
RTREE_TEMPLATE
typename RTREE_QUAL::ListNode* RTREE_QUAL::AllocListNode()
{
#ifdef RTREE_DONT_USE_MEMPOOLS
  return new ListNode;
#else // RTREE_DONT_USE_MEMPOOLS
  // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
}


RTREE_TEMPLATE
void RTREE_QUAL::FreeListNode(ListNode* a_listNode)
{
#ifdef RTREE_DONT_USE_MEMPOOLS
  delete a_listNode;
#else // RTREE_DONT_USE_MEMPOOLS
  // EXAMPLE
#endif // RTREE_DONT_USE_MEMPOOLS
}


RTREE_TEMPLATE
void RTREE_QUAL::InitNode(Node* a_node)
{
  a_node->m_count = 0;
  a_node->m_level = -1;
}


RTREE_TEMPLATE
void RTREE_QUAL::InitRect(Rect* a_rect)
{
  for(int index = 0; index < NUMDIMS; ++index)
  {
    a_rect->m_min[index] = (ELEMTYPE)0;
    a_rect->m_max[index] = (ELEMTYPE)0;
  }
}


// Inserts a new data rectangle into the index structure.
// Recursively descends tree, propagates splits back up.
// Returns 0 if node was not split.  Old node updated.
// If node was split, returns 1 and sets the pointer pointed to by
// new_node to point to the new node.  Old node updated to become one of two.
// The level argument specifies the number of steps up from the leaf
// level to insert; e.g. a data rectangle goes in at level = 0.
RTREE_TEMPLATE
bool RTREE_QUAL::InsertRectRec(Rect* a_rect, const DATATYPE& a_id, Node* a_node, Node** a_newNode, int a_level)
{
  ASSERT(a_rect && a_node && a_newNode);
  ASSERT(a_level >= 0 && a_level <= a_node->m_level);

  int index;
  Branch branch;
  Node* otherNode;

  // Still above level for insertion, go down tree recursively
  if(a_node->m_level > a_level)
  {
    index = PickBranch(a_rect, a_node);
    if (!InsertRectRec(a_rect, a_id, a_node->m_branch[index].m_child, &otherNode, a_level))
    {
      // Child was not split
      a_node->m_branch[index].m_rect = CombineRect(a_rect, &(a_node->m_branch[index].m_rect));
      return false;
    }
    else // Child was split
    {
      a_node->m_branch[index].m_rect = NodeCover(a_node->m_branch[index].m_child);
      branch.m_child = otherNode;
      branch.m_rect = NodeCover(otherNode);
      return AddBranch(&branch, a_node, a_newNode);
    }
  }
  else if(a_node->m_level == a_level) // Have reached level for insertion. Add rect, split if necessary
  {
    branch.m_rect = *a_rect;
    branch.m_child = (Node*) a_id;
    // Child field of leaves contains id of data record
    return AddBranch(&branch, a_node, a_newNode);
  }
  else
  {
    // Should never occur
    ASSERT(0);
    return false;
  }
}

RTREE_TEMPLATE
bool RTREE_QUAL::InsertPointPot(Point* a_point, const DATATYPE& a_id, Node* a_node, Node** a_newNode, int a_level)
 {
	 ASSERT(a_point && a_node && a_newNode);
	 ASSERT(a_level >= 0 && a_level <= a_node->m_level);

	 int index;
	 Branch branch;
	 Node* otherNode;

	 // Still above level for insertion, go down tree recursively
	 if(a_node->m_level > a_level)
	 {
		 index = PickBranch(a_point, a_node);
		 // pickBranch fail, need split
		 if (index == -1)
		 {
			 Node* uselessNode;
			 if (ReorganizeNode(a_point, a_node, a_newNode, &index))
			 {
				 // Child was split
				 ASSERT(!InsertPointPot(a_point, a_id, a_node->m_branch[index].m_child, &uselessNode, a_level));
				 a_node->m_branch[index].m_rect = NodeCover(a_node->m_branch[index].m_child);
				 return true;
			 }
			 else
			 {
				ASSERT(!InsertPointPot(a_point, a_id, a_node->m_branch[index].m_child, &uselessNode, a_level));
				a_node->m_branch[index].m_rect = EnlargeRect(a_point, &(a_node->m_branch[index].m_rect));
				return false;
			 }
		 }
		 else
		 {
			 if (!InsertPointPot(a_point, a_id, a_node->m_branch[index].m_child, &otherNode, a_level))
			 {
				 // Child was not split
				 a_node->m_branch[index].m_rect = EnlargeRect(a_point, &(a_node->m_branch[index].m_rect));
				 return false;
			 }
			 else // Child was split
			 {
				 a_node->m_branch[index].m_rect = NodeCover(a_node->m_branch[index].m_child);
				 branch.m_child = otherNode;
				 branch.m_rect = NodeCover(otherNode);
				 return NewAddBranch(&branch, a_node, a_newNode);
			 }
		 }
	 }
	 else if(a_node->m_level == a_level) // Have reached level for insertion. Add rect, split if necessary
	 {
		 Rect rect;
		 for(int axis=0; axis<NUMDIMS; ++axis)
		 {
			 rect.m_min[axis] = a_point->m[axis];
			 rect.m_max[axis] = a_point->m[axis];
		 }
		 branch.m_rect = rect;
		 branch.m_child = (Node*) a_id;
		 // Child field of leaves contains id of data record
		 return NewAddBranch(&branch, a_node, a_newNode);
	 }
	 else
	 {
		 // Should never occur
		 ASSERT(0);
		 return false;
	 }
 }


// Insert a data rectangle into an index structure.
// InsertRect provides for splitting the root;
// returns 1 if root was split, 0 if it was not.
// The level argument specifies the number of steps up from the leaf
// level to insert; e.g. a data rectangle goes in at level = 0.
// InsertRect2 does the recursion.
//
RTREE_TEMPLATE
bool RTREE_QUAL::InsertRect(Rect* a_rect, const DATATYPE& a_id, Node** a_root, int a_level)
{
  ASSERT(a_rect && a_root);
  ASSERT(a_level >= 0 && a_level <= (*a_root)->m_level);
#ifdef _DEBUG
  for(int index=0; index < NUMDIMS; ++index)
  {
    ASSERT(a_rect->m_min[index] <= a_rect->m_max[index]);
  }
#endif //_DEBUG  

  Node* newRoot;
  Node* newNode;
  Branch branch;

  if(InsertRectRec(a_rect, a_id, *a_root, &newNode, a_level))  // Root split
  {
    newRoot = AllocNode();  // Grow tree taller and new root
    newRoot->m_level = (*a_root)->m_level + 1;
    branch.m_rect = NodeCover(*a_root);
    branch.m_child = *a_root;
    AddBranch(&branch, newRoot, NULL);
    branch.m_rect = NodeCover(newNode);
    branch.m_child = newNode;
    AddBranch(&branch, newRoot, NULL);
    *a_root = newRoot;
    return true;
  }

  return false;
}


RTREE_TEMPLATE
bool RTREE_QUAL::InsertPoint(Point* a_point, const DATATYPE& a_id, Node** a_root, int a_level)
{
	ASSERT(a_point && a_root);
	ASSERT(a_level >= 0 && a_level <= (*a_root)->m_level);

	Node* newRoot;
	Node* newNode;
	Branch branch;

	if(InsertPointPot(a_point, a_id, *a_root, &newNode, a_level))  // Root split
	{
		newRoot = AllocNode();  // Grow tree taller and new root
		newRoot->m_level = (*a_root)->m_level + 1;
		branch.m_rect = NodeCover(*a_root);
		branch.m_child = *a_root;
		NewAddBranch(&branch, newRoot, NULL);
		branch.m_rect = NodeCover(newNode);
		branch.m_child = newNode;
		NewAddBranch(&branch, newRoot, NULL);
		*a_root = newRoot;
		return true;
	}

	return false;
}


// Find the smallest rectangle that includes all rectangles in branches of a node.
RTREE_TEMPLATE
typename RTREE_QUAL::Rect RTREE_QUAL::NodeCover(Node* a_node)
{
  ASSERT(a_node);
  
  int firstTime = true;
  Rect rect;
  InitRect(&rect);
  
  for(int index = 0; index < a_node->m_count; ++index)
  {
    if(firstTime)
    {
      rect = a_node->m_branch[index].m_rect;
      firstTime = false;
    }
    else
    {
      rect = CombineRect(&rect, &(a_node->m_branch[index].m_rect));
    }
  }
  
  return rect;
}


// Add a branch to a node.  Split the node if necessary.
// Returns 0 if node not split.  Old node updated.
// Returns 1 if node split, sets *new_node to  address of new node.
// Old node updated, becomes one of two.
RTREE_TEMPLATE
bool RTREE_QUAL::AddBranch(Branch* a_branch, Node* a_node, Node** a_newNode)
{
  ASSERT(a_branch);
  ASSERT(a_node);

  if(a_node->m_count < MAXNODES)  // Split won't be necessary
  {
    a_node->m_branch[a_node->m_count] = *a_branch;
    ++a_node->m_count;

    return false;
  }
  else
  {
    ASSERT(a_newNode);
    
    SplitNode(a_node, a_branch, a_newNode);
    return true;
  }
}


RTREE_TEMPLATE
bool RTREE_QUAL::NewAddBranch(Branch* a_branch, Node* a_node, Node** a_newNode)
{
	ASSERT(a_branch);
	ASSERT(a_node);

	if(a_node->m_count < MAXNODES)  // Split won't be necessary
	{
		a_node->m_branch[a_node->m_count] = *a_branch;
		++a_node->m_count;

		return false;
	}
	else
	{
		ASSERT(a_newNode);

 		NewSplitNode(a_node, a_branch, a_newNode);
		return true;
	}
}


// Disconnect a dependent node.
// Caller must return (or stop using iteration index) after this as count has changed
RTREE_TEMPLATE
void RTREE_QUAL::DisconnectBranch(Node* a_node, int a_index)
{
  ASSERT(a_node && (a_index >= 0) && (a_index < MAXNODES));
  ASSERT(a_node->m_count > 0);

  // Remove element by swapping with the last element to prevent gaps in array
  a_node->m_branch[a_index] = a_node->m_branch[a_node->m_count - 1];
  
  --a_node->m_count;
}


// Pick a branch.  Pick the one that will need the smallest increase
// in area to accomodate the new rectangle.  This will result in the
// least total area for the covering rectangles in the current node.
// In case of a tie, pick the one which was smaller before, to get
// the best resolution when searching.
RTREE_TEMPLATE
int RTREE_QUAL::PickBranch(Rect* a_rect, Node* a_node)
{
  ASSERT(a_rect && a_node);
  
  bool firstTime = true;
  ELEMTYPEREAL increase;
  ELEMTYPEREAL bestIncr = (ELEMTYPEREAL)-1;
  ELEMTYPEREAL area;
  ELEMTYPEREAL bestArea;
  int best;
  Rect tempRect;

  for(int index=0; index < a_node->m_count; ++index)
  {
    Rect* curRect = &a_node->m_branch[index].m_rect;
    area = CalcRectVolume(curRect);
    tempRect = CombineRect(a_rect, curRect);
    increase = CalcRectVolume(&tempRect) - area;
    if((increase < bestIncr) || firstTime)
    {
      best = index;
      bestArea = area;
      bestIncr = increase;
      firstTime = false;
    }
    else if((increase == bestIncr) && (area < bestArea))
    {
      best = index;
      bestArea = area;
      bestIncr = increase;
    }
  }
  return best;
}

RTREE_TEMPLATE
int RTREE_QUAL::PickBranchSimple(Point* a_point, Node* a_node)
{
	ASSERT(a_point && a_node);

	bool firstTime = true;
	ELEMTYPEREAL increase;
	ELEMTYPEREAL bestIncr = (ELEMTYPEREAL)-1;
	ELEMTYPEREAL area;
	ELEMTYPEREAL bestArea;
	int best;
	Rect tempRect;

	for(int index=0; index < a_node->m_count; ++index)
	{
		Rect* curRect = &a_node->m_branch[index].m_rect;
		area = CalcRectVolume(curRect);
		tempRect = EnlargeRect(a_point, curRect);
		increase = CalcRectVolume(&tempRect) - area;
		if((increase < bestIncr) || firstTime)
		{
			best = index;
			bestArea = area;
			bestIncr = increase;
			firstTime = false;
		}
		else if((increase == bestIncr) && (area < bestArea))
		{
			best = index;
			bestArea = area;
			bestIncr = increase;
		}
	}
	return best;
}


RTREE_TEMPLATE
int RTREE_QUAL::PickBranch(Point* a_point, Node* a_node)
{
	ASSERT(a_point && a_node);

	bool firstTime = true;
	ELEMTYPEREAL increase;
	ELEMTYPEREAL bestIncr = (ELEMTYPEREAL)-1;
	ELEMTYPEREAL area;
	ELEMTYPEREAL bestArea;
	int best = -1;
	Rect tempRect;
	Rect* curRect;
	bool isOverlap;

	for(int index=0; index < a_node->m_count; ++index)
	{
		if (OverlapPoint(a_point, &a_node->m_branch[index].m_rect))
		{
			return index;
		}
		curRect = &a_node->m_branch[index].m_rect;
		area = CalcRectVolume(curRect);
		tempRect = EnlargeRect(a_point, curRect);
		increase = CalcRectVolume(&tempRect) - area;
		isOverlap = false;
		for (int i =0;i < a_node->m_count;i++)
		{
			if(i == index) continue;
			if (Overlap(&tempRect, &a_node->m_branch[i].m_rect))
			{
				isOverlap  = true;
				break;
			}
		}

		if(((increase < bestIncr) || firstTime) && !isOverlap)
		{
			best = index;
			bestArea = area;
			bestIncr = increase;
			firstTime = false;
		}
		else if((increase == bestIncr) && (area < bestArea) && !isOverlap)
		{
			best = index;
			bestArea = area;
			bestIncr = increase;
		}
	}
	
	return best;
}


RTREE_TEMPLATE
bool RTREE_QUAL::ReorganizeNode(Point* a_point, Node* a_node, Node** a_newNode, int* index)
{
	ASSERT(a_point && a_node);

	*index = PickBranchSimple(a_point, a_node);
	Rect tempRect;

	int  j = 0;
	bool isSplit = false;
	int bestPartition;
	double bestBorder;
	tempRect = EnlargeRect(a_point, &a_node->m_branch[*index].m_rect);
	for (int i = 0;i < a_node->m_count ;i++)
	{
		if (i == *index)
		{
			continue;
		}
		if (Overlap(&a_node->m_branch[i].m_rect, &tempRect))
		{

			ChoosePartitionValue(a_point, tempRect, a_node->m_branch[i].m_rect, &bestBorder, &bestPartition);
			if (a_node->m_count  < MAXNODES)
			{
				//split branch
				BranchArray branchArray;
				GetBranches(a_node->m_branch[*index].m_child, NULL, &branchArray);
				Branch branch;
				branch.m_child =  AllocNode();
				branch.m_child->m_level =
					a_node->m_branch[*index].m_child->m_level = a_node->m_level - 1;

				Partition(&branchArray,a_node->m_branch[*index].m_child, branch.m_child, 
					bestPartition, bestBorder);

				if (a_node->m_branch[*index].m_child->m_count == 0)
				{
					for(int axis=0; axis<NUMDIMS; ++axis)
					{
						a_node->m_branch[*index].m_rect.m_min[axis] = a_point->m[axis];
						a_node->m_branch[*index].m_rect.m_max[axis] = a_point->m[axis];
					}
					setNode(a_node->m_branch[*index].m_child, a_point);
				}
				else
					a_node->m_branch[*index].m_rect = NodeCover(a_node->m_branch[*index].m_child);
				if (branch.m_child->m_count == 0)
				{
					for(int axis=0; axis<NUMDIMS; ++axis)
					{
						branch.m_rect.m_min[axis] = a_point->m[axis];
						branch.m_rect.m_max[axis] = a_point->m[axis];
					}
					setNode(branch.m_child, a_point);
				}
				else
					branch.m_rect = NodeCover( branch.m_child);		
				NewAddBranch(&branch, a_node, NULL);
				if (a_point->m[bestPartition] > bestBorder)
				*index = a_node->m_count - 1;
				tempRect = a_node->m_branch[*index].m_rect = EnlargeRect(a_point, &a_node->m_branch[*index].m_rect);
			}
			else
			{
				//split node   rewrite newSplit
				isSplit = true;
				BranchArray branchArray1;
				int level = a_node->m_level;
				GetBranches(a_node, NULL, &branchArray1);
				*a_newNode = AllocNode();
				(*a_newNode)->m_level = a_node->m_level = level;
				
				if (a_point->m[bestPartition] < bestBorder)
				{
					Partition(&branchArray1, a_node, *a_newNode, bestPartition, bestBorder);
				}
				else
					Partition(&branchArray1,  *a_newNode, a_node, bestPartition, bestBorder);
				if (a_node->m_count == 0)
				{
					setNode(a_node, a_point);
					*index = 0;
					return isSplit;
				}
				else
				{
					*index = PickBranchSimple(a_point, a_node);
					tempRect = EnlargeRect(a_point, &a_node->m_branch[*index].m_rect);
				}
			}
		}
	}

	return isSplit;
}

RTREE_TEMPLATE
void RTREE_QUAL::setNode(Node* a_node, Point* a_point)
{
	if (a_node->m_level == 1)
	{
		a_node->m_count = 0;
		Branch branch;
		for(int axis=0; axis<NUMDIMS; ++axis)
		{
			branch.m_rect.m_min[axis] = a_point->m[axis];
			branch.m_rect.m_max[axis] = a_point->m[axis];
		}
		branch.m_child = AllocNode();
		branch.m_child->m_count = 0;
		branch.m_child->m_level = 0;
		NewAddBranch(&branch, a_node, NULL);
	}
	else if(a_node->m_level > 1)
	{
		a_node->m_count = 0;
		Branch branch;
		for(int axis=0; axis<NUMDIMS; ++axis)
		{
			branch.m_rect.m_min[axis] = a_point->m[axis];
			branch.m_rect.m_max[axis] = a_point->m[axis];
		}
		branch.m_child = AllocNode();
		branch.m_child->m_level = a_node->m_level-1;
		branch.m_child->m_count = 0;
		setNode(branch.m_child, a_point);
		NewAddBranch(&branch, a_node, NULL);
	}
}

RTREE_TEMPLATE
void RTREE_QUAL::ChoosePartitionValue(Point* a_point, Rect rectA, Rect rectB, double* bestBorder, int* bestPartition)
{
	double bestRatio, tempRatio, bestWidth, tempPartitionValue;
	bool firstTime = true;
	for (int i = 0;i < NUMDIMS; i++)
	{
		// possible
		if ((a_point->m[i] < rectB.m_min[i] || a_point->m[i] > rectB.m_max[i]) && (a_point->m[i] >= rectA.m_min[i] && a_point->m[i] <= rectA.m_max[i])) //
		{
			if ((rectA.m_min[i] < rectB.m_min[i] && rectA.m_max[i] > rectB.m_min[i])
			|| (rectA.m_min[i] < rectB.m_max[i] && rectA.m_max[i] > rectB.m_max[i]))
			{
				tempPartitionValue = a_point->m[i];
				if (a_point->m[i] < rectB.m_min[i] )
					tempPartitionValue += 0.0001;
				else
					tempPartitionValue -= 0.0001;

				// choose best
				tempRatio = (tempPartitionValue - rectA.m_min[i]) / ( rectA.m_max[i] - tempPartitionValue);
				if (tempRatio > 1)
					tempRatio = 1 / tempRatio;
				if (firstTime || (rectA.m_max[i] - rectA.m_min[i] > 1.1 * bestWidth && tempRatio >= 0.5))
				{
					bestRatio = tempRatio;
					bestWidth = rectA.m_max[i] - rectA.m_min[i];
					*bestBorder = tempPartitionValue;
					*bestPartition = i;
					firstTime = false;
				}
				else if(rectA.m_max[i] - rectA.m_min[i] >  bestWidth && tempRatio > bestRatio)
				{
					bestRatio = tempRatio;
					bestWidth = rectA.m_max[i] - rectA.m_min[i];
					*bestBorder = tempPartitionValue;
					*bestPartition  = i;
				}

				// another method
				if (a_point->m[i] < rectB.m_min[i] )
					tempPartitionValue = rectB.m_min[i] - 0.0001;
				else
					tempPartitionValue = rectB.m_max[i] + 0.0001;

				tempRatio = (tempPartitionValue - rectA.m_min[i]) / ( rectA.m_max[i] - tempPartitionValue);
				if (tempRatio > 1)
					tempRatio = 1 / tempRatio;
				if (firstTime || (rectA.m_max[i] - rectA.m_min[i] > 1.1 * bestWidth && tempRatio >= 0.5))
				{
					bestRatio = tempRatio;
					bestWidth = rectA.m_max[i] - rectA.m_min[i];
					*bestBorder = tempPartitionValue;
					*bestPartition  = i;
					firstTime = false;
				}
				else if(rectA.m_max[i] - rectA.m_min[i] >  bestWidth && tempRatio > bestRatio)
				{
					bestRatio = tempRatio;
					bestWidth = rectA.m_max[i] - rectA.m_min[i];
					*bestBorder = tempPartitionValue;
					*bestPartition  = i;
				}
			}
		}	
	}
}

// Combine two rectangles into larger one containing both
RTREE_TEMPLATE
typename RTREE_QUAL::Rect RTREE_QUAL::CombineRect(Rect* a_rectA, Rect* a_rectB)
{
  ASSERT(a_rectA && a_rectB);

  Rect newRect;

  for(int index = 0; index < NUMDIMS; ++index)
  {
    newRect.m_min[index] = Min(a_rectA->m_min[index], a_rectB->m_min[index]);
    newRect.m_max[index] = Max(a_rectA->m_max[index], a_rectB->m_max[index]);
  }

  return newRect;
}


RTREE_TEMPLATE
typename RTREE_QUAL::Rect RTREE_QUAL::EnlargeRect(Point* a_point, Rect* a_rectB)
{
	ASSERT(a_point && a_rectB);

	Rect newRect;

	for(int index = 0; index < NUMDIMS; ++index)
	{
		newRect.m_min[index] = Min(a_point->m[index], a_rectB->m_min[index]);
		newRect.m_max[index] = Max(a_point->m[index], a_rectB->m_max[index]);
	}

	return newRect;
}


// Split a node.
// Divides the nodes branches and the extra one between two nodes.
// Old node is one of the new ones, and one really new one is created.
// Tries more than one method for choosing a partition, uses best result.
RTREE_TEMPLATE
void RTREE_QUAL::SplitNode(Node* a_node, Branch* a_branch, Node** a_newNode)
{
  ASSERT(a_node);
  ASSERT(a_branch);

  // Could just use local here, but member or external is faster since it is reused
  PartitionVars localVars;
  PartitionVars* parVars = &localVars;
  int level;

  // Load all the branches into a buffer, initialize old node
  level = a_node->m_level;
  GetBranches(a_node, a_branch, parVars);

  // Find partition
  ChoosePartition(parVars, MINNODES);

  // Put branches from buffer into 2 nodes according to chosen partition
  *a_newNode = AllocNode();
  (*a_newNode)->m_level = a_node->m_level = level;
  LoadNodes(a_node, *a_newNode, parVars);
  
  ASSERT((a_node->m_count + (*a_newNode)->m_count) == parVars->m_total);
}


RTREE_TEMPLATE
void RTREE_QUAL::NewSplitNode(Node* a_node, Branch* a_branch, Node** a_newNode)
{
	ASSERT(a_node);
	//ASSERT(a_branch);

	// Could just use local here, but member or external is faster since it is reused
	BranchArray branchArray;
	int level;

	// Load all the branches into a buffer, initialize old node
	level = a_node->m_level;
	GetBranches(a_node, a_branch, &branchArray);

	// Find partition Rect
	*a_newNode = AllocNode();
	(*a_newNode)->m_level = a_node->m_level = level;
	ChoosePartition(&branchArray,   a_node, *a_newNode);
}

RTREE_TEMPLATE
void RTREE_QUAL::GetBranches(Node* a_node, Branch* a_branch, BranchArray* branchArray)
{
	ASSERT(a_node);

	//ASSERT(a_node->m_count == MAXNODES);

	// Load the branch buffer
	for(int index=0; index < a_node->m_count; ++index)
	{
		branchArray->m_branchBuf[index] = a_node->m_branch[index];
	}
	branchArray->m_count = a_node->m_count;
	if (a_branch != NULL)
	{
		branchArray->m_branchBuf[a_node->m_count] = *a_branch;
		branchArray->m_count ++;
	}
	InitNode(a_node);
}

RTREE_TEMPLATE
void RTREE_QUAL:: ChoosePartition(BranchArray* branchArray, Node* a_node, Node* a_newNode)
{
	int i, j;
	int splitNumber, bestSplitNumber, bestPatition, coverNum = MINNODES;
	bool firstTime = true;
	double border, bottom, bestDistance, bestBorder;

	// 选取最佳分割维度
	for (i = 0;i < NUMDIMS;i++)
	{
		splitNumber = 0;
		coverNum=MINNODES;
		sortAssistNumByAxis(branchArray, i);
		bottom = branchArray->m_sortNumber[branchArray->m_sortIndex[0]];
		border = branchArray->m_assistSortNumber[branchArray->m_sortIndex[0]];
		for (j = 1;j < MINNODES;j++)
		{
			border = Max(border, branchArray->m_assistSortNumber[branchArray->m_sortIndex[j]]);
		}
		for (;j < branchArray->m_count;j++)
		{
			//if (branchArray->m_sortNumber[branchArray->m_sortIndex[j]] > border)
			//{
			//	break;
			//}
			if (branchArray->m_sortNumber[branchArray->m_sortIndex[j]] <= border)
			{
				splitNumber++;
			}
			if (branchArray->m_assistSortNumber[branchArray->m_sortIndex[j]] <= border)
			{
				coverNum++;
			}
		}
		if (coverNum > MAXNODES)
		{
			continue;
		}
		if ((firstTime || splitNumber < bestSplitNumber))//最大的min不能比border宽 branchArray->m_sortNumber[branchArray->m_sortIndex[branchArray->m_count]] > border
		{
			bestPatition = i;
			bestSplitNumber = splitNumber;
			bestDistance = border - bottom;
			bestBorder = border;
			firstTime = false;
		}
		else if (splitNumber == bestSplitNumber && (border - bottom) > bestDistance )
		{
			bestPatition = i;
			bestDistance = border - bottom;
			bestBorder = border;
		}
	}	

	Partition(branchArray, a_node, a_newNode, bestPatition, bestBorder);
}

RTREE_TEMPLATE
void RTREE_QUAL::Partition(BranchArray* branchArray, Node* a_node, Node* a_newNode, int bestPatition, double bestBorder)
{
	Branch branch;
	sortByAxis(branchArray, bestPatition);
	for (int i = 0;i < branchArray->m_count;i++)
	{
		if (branchArray->m_assistSortNumber[branchArray->m_sortIndex[i]] <= bestBorder)
		{
			NewAddBranch(&branchArray->m_branchBuf[branchArray->m_sortIndex[i]], a_node, NULL);
		}
		else if (branchArray->m_sortNumber[branchArray->m_sortIndex[i]] > bestBorder)
		{
			NewAddBranch(&branchArray->m_branchBuf[branchArray->m_sortIndex[i]], a_newNode, NULL);
		}
		else
		{
			BranchArray bra;
			GetBranches(branchArray->m_branchBuf[branchArray->m_sortIndex[i]].m_child, NULL, &bra);
			branch.m_child =  AllocNode();
			branch.m_child->m_level =
				branchArray->m_branchBuf[branchArray->m_sortIndex[i]].m_child->m_level = a_node->m_level - 1;
			//branch.m_rect = branchArray->m_branchBuf[branchArray->m_sortIndex[i]].m_rect;

			//sortByAxis(&bra, bestPatition);
			//if (bra.m_sortNumber[0] == bra.m_assistSortNumber[0])
			//{
			//	for (int j = 0;j < bra.m_count;j++)
			//	{
			//		if (bra.m_sortNumber[bra.m_sortIndex[j]] < bestBorder)
			//		{
			//			branchArray->m_branchBuf[branchArray->m_sortIndex[i]].m_rect.m_max[bestPatition] = bra.m_sortNumber[bra.m_sortIndex[j]];
			//		}
			//		else
			//		{
			//			 branch.m_rect.m_min[bestPatition]= bra.m_sortNumber[bra.m_sortIndex[j]];
			//			break;
			//		}
			//	}
			//}
			//else
			//{
			//	branch.m_rect.m_min[bestPatition] = bestBorder+ 0.0000001;
			//	branchArray->m_branchBuf[branchArray->m_sortIndex[i]].m_rect.m_max[bestPatition] = bestBorder;
			//}


			Partition(&bra,branchArray->m_branchBuf[branchArray->m_sortIndex[i]].m_child, branch.m_child, 
				bestPatition, bestBorder);
			branchArray->m_branchBuf[branchArray->m_sortIndex[i]].m_rect = NodeCover(branchArray->m_branchBuf[branchArray->m_sortIndex[i]].m_child);
			branch.m_rect = NodeCover( branch.m_child);
			NewAddBranch(&branchArray->m_branchBuf[branchArray->m_sortIndex[i]], a_node, NULL);
			NewAddBranch(&branch, a_newNode, NULL);
		}
	}
}

RTREE_TEMPLATE
void RTREE_QUAL::sortByAxis(BranchArray* branchArray, int partitionAxis)
{
	int i, j;
	for (i = 0;i < branchArray->m_count;i++)
	{
		if (branchArray->m_branchBuf[i].m_rect.m_max[partitionAxis] > branchArray->m_branchBuf[i].m_rect.m_min[partitionAxis])
		{
			branchArray->m_sortNumber[i] = branchArray->m_branchBuf[i].m_rect.m_min[partitionAxis];
			branchArray->m_assistSortNumber[i] = branchArray->m_branchBuf[i].m_rect.m_max[partitionAxis];
		}
		else
		{
			branchArray->m_sortNumber[i] = branchArray->m_branchBuf[i].m_rect.m_max[partitionAxis];
			branchArray->m_assistSortNumber[i] = branchArray->m_branchBuf[i].m_rect.m_min[partitionAxis];
		}
	}
	double best, lastBest = -1;
	int bestNum;
	bool firstTime = true;
	bool *visited = new bool[MAXNODES+1];
	for (i = 0;i < MAXNODES+1;i++)
	{
		visited[i] = false;
	}

	for (i = 0;i < branchArray->m_count;i++)
	{
		for (j = 0;j < branchArray->m_count;j++)
		{
			if ((firstTime || branchArray->m_sortNumber[j] < best) && 
				(branchArray->m_sortNumber[j] > lastBest || (branchArray->m_sortNumber[j] == lastBest && visited[j] == false)))
			{
					best = branchArray->m_sortNumber[j];
					bestNum = j;
					firstTime = false;
			}
		}
		firstTime = true;
		lastBest = best;
		visited[bestNum] = true;
		branchArray->m_sortIndex[i] = bestNum;
	}
	for (i = 0;i < branchArray->m_count-1;i++)
	{
		if (branchArray->m_sortIndex[i] == branchArray->m_sortIndex[branchArray->m_count-1])
		{
			break;
		}
	}
}

RTREE_TEMPLATE
void RTREE_QUAL::sortAssistNumByAxis(BranchArray* branchArray, int partitionAxis)
{
	int i, j;
	for (i = 0;i < branchArray->m_count;i++)
	{
		if (branchArray->m_branchBuf[i].m_rect.m_max[partitionAxis] > branchArray->m_branchBuf[i].m_rect.m_min[partitionAxis])
		{
			branchArray->m_sortNumber[i] = branchArray->m_branchBuf[i].m_rect.m_min[partitionAxis];
			branchArray->m_assistSortNumber[i] = branchArray->m_branchBuf[i].m_rect.m_max[partitionAxis];
		}
		else
		{
			branchArray->m_sortNumber[i] = branchArray->m_branchBuf[i].m_rect.m_max[partitionAxis];
			branchArray->m_assistSortNumber[i] = branchArray->m_branchBuf[i].m_rect.m_min[partitionAxis];
		}
	}
	double best, lastBest = -1;
	int bestNum;
	bool firstTime = true;
	bool *visited = new bool[MAXNODES+1];
	for (i = 0;i < MAXNODES+1;i++)
	{
		visited[i] = false;
	}

	for (i = 0;i < branchArray->m_count;i++)
	{
		for (j = 0;j < branchArray->m_count;j++)
		{
			if ((firstTime || branchArray->m_assistSortNumber[j] < best) && 
				(branchArray->m_assistSortNumber[j] > lastBest || (branchArray->m_assistSortNumber[j] == lastBest && visited[j] == false)))
			{
				best = branchArray->m_assistSortNumber[j];
				bestNum = j;
				firstTime = false;
			}
		}
		firstTime = true;
		lastBest = best;
		visited[bestNum] = true;
		branchArray->m_sortIndex[i] = bestNum;
	}
	for (i = 0;i < branchArray->m_count-1;i++)
	{
		if (branchArray->m_sortIndex[i] == branchArray->m_sortIndex[branchArray->m_count-1])
		{
			break;
		}
	}
}


// Calculate the n-dimensional volume of a rectangle
RTREE_TEMPLATE
ELEMTYPEREAL RTREE_QUAL::RectVolume(Rect* a_rect)
{
  ASSERT(a_rect);
  
  ELEMTYPEREAL volume = (ELEMTYPEREAL)1;

  for(int index=0; index<NUMDIMS; ++index)
  {
    volume *= a_rect->m_max[index] - a_rect->m_min[index];
  }
  
  ASSERT(volume >= (ELEMTYPEREAL)0);
  
  return volume;
}


// The exact volume of the bounding sphere for the given Rect
RTREE_TEMPLATE
ELEMTYPEREAL RTREE_QUAL::RectSphericalVolume(Rect* a_rect)
{
  ASSERT(a_rect);
   
  ELEMTYPEREAL sumOfSquares = (ELEMTYPEREAL)0;
  ELEMTYPEREAL radius;

  for(int index=0; index < NUMDIMS; ++index) 
  {
    ELEMTYPEREAL halfExtent = ((ELEMTYPEREAL)a_rect->m_max[index] - (ELEMTYPEREAL)a_rect->m_min[index]) * 0.5f;
    sumOfSquares += halfExtent * halfExtent;
  }

  radius = (ELEMTYPEREAL)sqrt(sumOfSquares);
  
  // Pow maybe slow, so test for common dims like 2,3 and just use x*x, x*x*x.
  if(NUMDIMS == 3)
  {
    return (radius * radius * radius * m_unitSphereVolume);
  }
  else if(NUMDIMS == 2)
  {
    return (radius * radius * m_unitSphereVolume);
  }
  else
  {
    return (ELEMTYPEREAL)(pow(radius, NUMDIMS) * m_unitSphereVolume);
  }
}


// Use one of the methods to calculate retangle volume
RTREE_TEMPLATE
ELEMTYPEREAL RTREE_QUAL::CalcRectVolume(Rect* a_rect)
{
#ifdef RTREE_USE_SPHERICAL_VOLUME
  return RectSphericalVolume(a_rect); // Slower but helps certain merge cases
#else // RTREE_USE_SPHERICAL_VOLUME
  return RectVolume(a_rect); // Faster but can cause poor merges
#endif // RTREE_USE_SPHERICAL_VOLUME  
}


// Load branch buffer with branches from full node plus the extra branch.
RTREE_TEMPLATE
void RTREE_QUAL::GetBranches(Node* a_node, Branch* a_branch, PartitionVars* a_parVars)
{
  ASSERT(a_node);
  ASSERT(a_branch);

  ASSERT(a_node->m_count == MAXNODES);
    
  // Load the branch buffer
  for(int index=0; index < MAXNODES; ++index)
  {
    a_parVars->m_branchBuf[index] = a_node->m_branch[index];
  }
  a_parVars->m_branchBuf[MAXNODES] = *a_branch;
  a_parVars->m_branchCount = MAXNODES + 1;

  // Calculate rect containing all in the set
  a_parVars->m_coverSplit = a_parVars->m_branchBuf[0].m_rect;
  for(int index=1; index < MAXNODES+1; ++index)
  {
    a_parVars->m_coverSplit = CombineRect(&a_parVars->m_coverSplit, &a_parVars->m_branchBuf[index].m_rect);
  }
  a_parVars->m_coverSplitArea = CalcRectVolume(&a_parVars->m_coverSplit);

  InitNode(a_node);
}


// Method #0 for choosing a partition:
// As the seeds for the two groups, pick the two rects that would waste the
// most area if covered by a single rectangle, i.e. evidently the worst pair
// to have in the same group.
// Of the remaining, one at a time is chosen to be put in one of the two groups.
// The one chosen is the one with the greatest difference in area expansion
// depending on which group - the rect most strongly attracted to one group
// and repelled from the other.
// If one group gets too full (more would force other group to violate min
// fill requirement) then other group gets the rest.
// These last are the ones that can go in either group most easily.
RTREE_TEMPLATE
void RTREE_QUAL::ChoosePartition(PartitionVars* a_parVars, int a_minFill)
{
  ASSERT(a_parVars);
  
  ELEMTYPEREAL biggestDiff;
  int group, chosen, betterGroup;
  
  InitParVars(a_parVars, a_parVars->m_branchCount, a_minFill);
  PickSeeds(a_parVars);

  while (((a_parVars->m_count[0] + a_parVars->m_count[1]) < a_parVars->m_total)
       && (a_parVars->m_count[0] < (a_parVars->m_total - a_parVars->m_minFill))
       && (a_parVars->m_count[1] < (a_parVars->m_total - a_parVars->m_minFill)))
  {
    biggestDiff = (ELEMTYPEREAL) -1;
    for(int index=0; index<a_parVars->m_total; ++index)
    {
      if(!a_parVars->m_taken[index])
      {
        Rect* curRect = &a_parVars->m_branchBuf[index].m_rect;
        Rect rect0 = CombineRect(curRect, &a_parVars->m_cover[0]);
        Rect rect1 = CombineRect(curRect, &a_parVars->m_cover[1]);
        ELEMTYPEREAL growth0 = CalcRectVolume(&rect0) - a_parVars->m_area[0];
        ELEMTYPEREAL growth1 = CalcRectVolume(&rect1) - a_parVars->m_area[1];
        ELEMTYPEREAL diff = growth1 - growth0;
        if(diff >= 0)
        {
          group = 0;
        }
        else
        {
          group = 1;
          diff = -diff;
        }

        if(diff > biggestDiff)
        {
          biggestDiff = diff;
          chosen = index;
          betterGroup = group;
        }
        else if((diff == biggestDiff) && (a_parVars->m_count[group] < a_parVars->m_count[betterGroup]))
        {
          chosen = index;
          betterGroup = group;
        }
      }
    }
    Classify(chosen, betterGroup, a_parVars);
  }

  // If one group too full, put remaining rects in the other
  if((a_parVars->m_count[0] + a_parVars->m_count[1]) < a_parVars->m_total)
  {
    if(a_parVars->m_count[0] >= a_parVars->m_total - a_parVars->m_minFill)
    {
      group = 1;
    }
    else
    {
      group = 0;
    }
    for(int index=0; index<a_parVars->m_total; ++index)
    {
      if(!a_parVars->m_taken[index])
      {
        Classify(index, group, a_parVars);
      }
    }
  }

  ASSERT((a_parVars->m_count[0] + a_parVars->m_count[1]) == a_parVars->m_total);
  ASSERT((a_parVars->m_count[0] >= a_parVars->m_minFill) && 
        (a_parVars->m_count[1] >= a_parVars->m_minFill));
}


// Copy branches from the buffer into two nodes according to the partition.
RTREE_TEMPLATE
void RTREE_QUAL::LoadNodes(Node* a_nodeA, Node* a_nodeB, PartitionVars* a_parVars)
{
  ASSERT(a_nodeA);
  ASSERT(a_nodeB);
  ASSERT(a_parVars);

  for(int index=0; index < a_parVars->m_total; ++index)
  {
    ASSERT(a_parVars->m_partition[index] == 0 || a_parVars->m_partition[index] == 1);
    
    if(a_parVars->m_partition[index] == 0)
    {
      AddBranch(&a_parVars->m_branchBuf[index], a_nodeA, NULL);
    }
    else if(a_parVars->m_partition[index] == 1)
    {
      AddBranch(&a_parVars->m_branchBuf[index], a_nodeB, NULL);
    }
  }
}


// Initialize a PartitionVars structure.
RTREE_TEMPLATE
void RTREE_QUAL::InitParVars(PartitionVars* a_parVars, int a_maxRects, int a_minFill)
{
  ASSERT(a_parVars);

  a_parVars->m_count[0] = a_parVars->m_count[1] = 0;
  a_parVars->m_area[0] = a_parVars->m_area[1] = (ELEMTYPEREAL)0;
  a_parVars->m_total = a_maxRects;
  a_parVars->m_minFill = a_minFill;
  for(int index=0; index < a_maxRects; ++index)
  {
    a_parVars->m_taken[index] = false;
    a_parVars->m_partition[index] = -1;
  }
}


RTREE_TEMPLATE
void RTREE_QUAL::PickSeeds(PartitionVars* a_parVars)
{
  int seed0, seed1;
  ELEMTYPEREAL worst, waste;
  ELEMTYPEREAL area[MAXNODES+1];

  for(int index=0; index<a_parVars->m_total; ++index)
  {
    area[index] = CalcRectVolume(&a_parVars->m_branchBuf[index].m_rect);
  }

  worst = -a_parVars->m_coverSplitArea - 1;
  for(int indexA=0; indexA < a_parVars->m_total-1; ++indexA)
  {
    for(int indexB = indexA+1; indexB < a_parVars->m_total; ++indexB)
    {
      Rect oneRect = CombineRect(&a_parVars->m_branchBuf[indexA].m_rect, &a_parVars->m_branchBuf[indexB].m_rect);
      waste = CalcRectVolume(&oneRect) - area[indexA] - area[indexB];
      if(waste > worst)
      {
        worst = waste;
        seed0 = indexA;
        seed1 = indexB;
      }
    }
  }
  Classify(seed0, 0, a_parVars);
  Classify(seed1, 1, a_parVars);
}


// Put a branch in one of the groups.
RTREE_TEMPLATE
void RTREE_QUAL::Classify(int a_index, int a_group, PartitionVars* a_parVars)
{
  ASSERT(a_parVars);
  ASSERT(!a_parVars->m_taken[a_index]);

  a_parVars->m_partition[a_index] = a_group;
  a_parVars->m_taken[a_index] = true;

  if (a_parVars->m_count[a_group] == 0)
  {
    a_parVars->m_cover[a_group] = a_parVars->m_branchBuf[a_index].m_rect;
  }
  else
  {
    a_parVars->m_cover[a_group] = CombineRect(&a_parVars->m_branchBuf[a_index].m_rect, &a_parVars->m_cover[a_group]);
  }
  a_parVars->m_area[a_group] = CalcRectVolume(&a_parVars->m_cover[a_group]);
  ++a_parVars->m_count[a_group];
}


// Delete a data rectangle from an index structure.
// Pass in a pointer to a Rect, the tid of the record, ptr to ptr to root node.
// Returns 1 if record not found, 0 if success.
// RemoveRect provides for eliminating the root.
RTREE_TEMPLATE
bool RTREE_QUAL::RemoveRect(Rect* a_rect, const DATATYPE& a_id, Node** a_root)
{
  ASSERT(a_rect && a_root);
  ASSERT(*a_root);

  Node* tempNode;
  ListNode* reInsertList = NULL;

  if(!RemoveRectRec(a_rect, a_id, *a_root, &reInsertList))
  {
    // Found and deleted a data item
    // Reinsert any branches from eliminated nodes
    while(reInsertList)
    {
      tempNode = reInsertList->m_node;

      for(int index = 0; index < tempNode->m_count; ++index)
      {
        InsertRect(&(tempNode->m_branch[index].m_rect),
                   tempNode->m_branch[index].m_data,
                   a_root,
                   tempNode->m_level);
      }
      
      ListNode* remLNode = reInsertList;
      reInsertList = reInsertList->m_next;
      
      FreeNode(remLNode->m_node);
      FreeListNode(remLNode);
    }
    
    // Check for redundant root (not leaf, 1 child) and eliminate
    if((*a_root)->m_count == 1 && (*a_root)->IsInternalNode())
    {
      tempNode = (*a_root)->m_branch[0].m_child;
      
      ASSERT(tempNode);
      FreeNode(*a_root);
      *a_root = tempNode;
    }
    return false;
  }
  else
  {
    return true;
  }
}

RTREE_TEMPLATE
bool RTREE_QUAL::RemovePoint(Point* a_point, const DATATYPE& a_id, Node** a_root)
{
	ASSERT(*a_root);

	Node* tempNode;
	ListNode* reInsertList = NULL;

	if (!RemovePointPoi(a_point, a_id, *a_root, &reInsertList))
	{
		while(reInsertList)
		{
			tempNode = reInsertList->m_node;

			for (int index=0; index<tempNode->m_count; index++)
			{
				//插入
			}

			ListNode* remLNode = reInsertList;
			reInsertList = reInsertList->m_next;

			FreeNode(remLNode->m_node);
			FreeListNode(remLNode);
		}

		if((*a_root)->m_count == 1 && (*a_root)->IsInternalNode())
		{
			tempNode = (*a_root)->m_branch[0].m_child;

			ASSERT(tempNode);
			FreeNode(*a_root);
			*a_root = tempNode;
		}
		return false;
	}
	else
	{
		return true;
	}

}

RTREE_TEMPLATE
bool RTREE_QUAL::RemovePointPoi(Point* a_point, const DATATYPE& a_id, Node* a_node, ListNode** a_listNode)
{
	ASSERT(a_node && a_listNode);
	ASSERT(a_node->m_level >= 0);

	if (node->IsInternalNode())
	{
		for (int index=0; index<a_node->m_count; index++)
		{
			if (OverlapPoint(a_point, &(a_node->m_branch[index].m_rect)))
			{
				if (!RemovePointPoi(a_point, a_id, a_node->m_branch[index].m_child, a_listNode))
				{
					if (a_node->m_branch[index].m_child->m_count >= MINNODES)
					{
						a_node->m_branch[index].m_rect = NodeCover(a_node->m_branch[index].m_child);
					}
					else
					{
						ReInsert(a_node->m_branch[index].m_child, a_listNode);
						DisconnectBranch(a_node, index);
					}
					return false;
				}
			}
		}
		return true;
	}
	else
	{
		for (int index=0; index<a_node->m_count; index++)
		{
			if (a_node->m_branch[index].m_child == (Node*)a_id)
			{
				DisconnectBranch(a_node, index); // Must return after this call as count has changed
				return false;
			}
		}
		return true;
	}
}

// Delete a rectangle from non-root part of an index structure.
// Called by RemoveRect.  Descends tree recursively,
// merges branches on the way back up.
// Returns 1 if record not found, 0 if success.
RTREE_TEMPLATE
bool RTREE_QUAL::RemoveRectRec(Rect* a_rect, const DATATYPE& a_id, Node* a_node, ListNode** a_listNode)
{
  ASSERT(a_rect && a_node && a_listNode);
  ASSERT(a_node->m_level >= 0);

  if(a_node->IsInternalNode())  // not a leaf node
  {
    for(int index = 0; index < a_node->m_count; ++index)
    {
      if(Overlap(a_rect, &(a_node->m_branch[index].m_rect)))
      {
        if(!RemoveRectRec(a_rect, a_id, a_node->m_branch[index].m_child, a_listNode))
        {
          if(a_node->m_branch[index].m_child->m_count >= MINNODES)
          {
            // child removed, just resize parent rect
            a_node->m_branch[index].m_rect = NodeCover(a_node->m_branch[index].m_child);
          }
          else
          {
            // child removed, not enough entries in node, eliminate node
            ReInsert(a_node->m_branch[index].m_child, a_listNode);
            DisconnectBranch(a_node, index); // Must return after this call as count has changed
          }
          return false;
        }
      }
    }
    return true;
  }
  else // A leaf node
  {
    for(int index = 0; index < a_node->m_count; ++index)
    {
      if(a_node->m_branch[index].m_child == (Node*)a_id)
      {
        DisconnectBranch(a_node, index); // Must return after this call as count has changed
        return false;
      }
    }
    return true;
  }
}


// Decide whether two rectangles overlap.
RTREE_TEMPLATE
bool RTREE_QUAL::Overlap(Rect* a_rectA, Rect* a_rectB)
{
  ASSERT(a_rectA && a_rectB);

  for(int index=0; index < NUMDIMS; ++index)
  {
    if (a_rectA->m_min[index] > a_rectB->m_max[index] ||
        a_rectB->m_min[index] > a_rectA->m_max[index])
    {
      return false;
    }
  }
  return true;
}

RTREE_TEMPLATE
bool RTREE_QUAL::OverlapPoint(Point* a_point, Rect* a_rectB)
{
	ASSERT(a_rectB);

	for(int index=0; index < NUMDIMS; ++index)
	{
		if (a_point->m[index] > a_rectB->m_max[index] ||
			a_rectB->m_min[index] > a_point->m[index])
		{
			return false;
		}
	}
	return true;
}


// Add a node to the reinsertion list.  All its branches will later
// be reinserted into the index structure.
RTREE_TEMPLATE
void RTREE_QUAL::ReInsert(Node* a_node, ListNode** a_listNode)
{
  ListNode* newListNode;

  newListNode = AllocListNode();
  newListNode->m_node = a_node;
  newListNode->m_next = *a_listNode;
  *a_listNode = newListNode;
}


// Search in an index tree or subtree for all data retangles that overlap the argument rectangle.
RTREE_TEMPLATE
bool RTREE_QUAL::Search(Node* a_node, Rect* a_rect, int& a_foundCount, bool __cdecl a_resultCallback(DATATYPE a_data, void* a_context), void* a_context)
{
  ASSERT(a_node);
  ASSERT(a_node->m_level >= 0);
  ASSERT(a_rect);

  if(a_node->IsInternalNode()) // This is an internal node in the tree
  {
    for(int index=0; index < a_node->m_count; ++index)
    {
      if(Overlap(a_rect, &a_node->m_branch[index].m_rect))
      {
        if(!Search(a_node->m_branch[index].m_child, a_rect, a_foundCount, a_resultCallback, a_context))
        {
	      visitCount++;
          return false; // Don't continue searching
        }
      }
    }
  }
  else // This is a leaf node
  {
    for(int index=0; index < a_node->m_count; ++index)
    {

      if(Overlap(a_rect, &a_node->m_branch[index].m_rect))
      {
        DATATYPE& id = a_node->m_branch[index].m_data;
        
        // NOTE: There are different ways to return results.  Here's where to modify
        if(&a_resultCallback)
        {
          ++a_foundCount;
          if(!a_resultCallback(id, a_context))
          {
			visitCount++;
            return false; // Don't continue searching
          }
        }
      }
    }
  }

  visitCount++;
  return true; // Continue searching
}

RTREE_TEMPLATE
ELEMTYPE RTREE_QUAL::minDist(Rect* a_rect, ELEMTYPE queryPoint[NUMDIMS])
{
	ASSERT(a_rect);
	ASSERT(queryPoint);
	ELEMTYPE distance = (ELEMTYPE)1;
	ELEMTYPE r;
	for(int index = 0; index < NUMDIMS; ++index)
	{
		if(queryPoint[index] > a_rect->m_max[index])
		{
			r = a_rect->m_max[index];
		}
		else if(queryPoint[index] < a_rect->m_min[index])
		{
			r = a_rect->m_min[index];
		}
		else
		{
			r = queryPoint[index];
		}
		distance += ((queryPoint[index] - r)/scale) *((queryPoint[index] - r)/scale);
	}
	return distance;
}

RTREE_TEMPLATE
DATATYPE* RTREE_QUAL::KNN(ELEMTYPE queryPoint[NUMDIMS], int k)
{
	ASSERT(k>=1);
	DATATYPE* idList = new DATATYPE[k];
	NeighbourNode* knnList = new NeighbourNode[k];
	ASSERT(knnList != NULL);
	int i;
	double max = (numeric_limits<double>::max)();   //随便取的值
	for(i = 0; i < k; i++)
	{
		knnList[i].distance = max;
	}
	KNearNeighbour(queryPoint,m_root, k, knnList, max);
	for(i = 0; i <k; i++)
		idList[i] = knnList[i].id;
	return idList;
}

RTREE_TEMPLATE
void RTREE_QUAL::KNearNeighbour(ELEMTYPE queryPoint[NUMDIMS], Node* a_node, int k, NeighbourNode* knnList, double &ndistance)
{
	ASSERT(a_node);
	ASSERT(knnList);
	if(a_node->IsInternalNode()) // This is an internal node in the tree
	{
		Branch activebranch[MAXNODES];   //对节点按照minMax进行排序,将大于ndistance的直接抛掉
		int activeCount = 1, i, index;
		ELEMTYPE mindist, tempdist;
		activebranch[0] = a_node->m_branch[0];

		//先对所有的分支进行一下排序，从近的分支开始找
		for(index=1; index < a_node->m_count; ++index)                
		{
			mindist = minDist(&(a_node->m_branch[index].m_rect), queryPoint);
			if(mindist >=ndistance)  //直接进行剪枝
				continue;
			//插入排序
			i = activeCount;
			while(i > 0)
			{
				tempdist = minDist(&(activebranch[i-1].m_rect),queryPoint);
				if(tempdist > mindist)
				{
					activebranch[i] = activebranch[i-1];
				}
				else
					break;
				i--;
			}
			activebranch[i] = a_node->m_branch[index];
			activeCount += 1;
		}//插入排序选择结束
		for(index = 0; index < activeCount; index ++)
		{
			mindist = minDist(&(activebranch[index].m_rect), queryPoint);
			if(mindist >=ndistance)  //后面的直接剪枝
				break;
			KNearNeighbour(queryPoint, activebranch[index].m_child, k, knnList, ndistance);  //递归
		}
	}
	else // This is a leaf node
	{
		int i, index, j;
		for(index=0; index < a_node->m_count; ++index)  //对节点中的每一个进行求值比较
		{
			double dist = 1.0;
			ELEMTYPE* point;
			point = a_node->m_branch[index].m_rect.m_min;
			for(i = 0; i < NUMDIMS; i++)
			{
				dist +=  ((point[i] - queryPoint[i])/scale) * ((point[i] - queryPoint[i])/scale);  //这里对范围进行了缩小的处理
			}
			if(dist >= ndistance)     //直接不比较，比如有相似度一样的k+1个点，只是随机取k个
				continue;
			//插入排序
			j = k - 1;
			while(j>0)
			{
				if(dist < knnList[j-1].distance)
				{
					knnList[j] = knnList[j-1];
				}
				else
					break;
				j--;
			}
			knnList[j].distance = dist;
			knnList[j].id = a_node->m_branch[index].m_data;
			ndistance = knnList[k-1].distance;
		}
	}


}


#undef RTREE_TEMPLATE
#undef RTREE_QUAL
#endif //RTREE_H