/*
Cache Simulator
Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss




struct config{
       int L1blocksize;
       int L1setsize;
       int L1size;
       int L2blocksize;
       int L2setsize;
       int L2size;
       };

/* you can define the cache class here, or design your own data structure for L1 and L2 cache
class cache {
      
      }
*/       
struct Address{
    unsigned int Tag;
    int Index;
    //unsigned long Offset;
    int Tagbits;
    int Indexbits;
    int Offsetbits;
    unsigned int Indexsize;
};

int *LRUaugment(int setsize, int *LRU) //make every LRU+1
{
    int *a=new int [setsize];
    for(int i=0;i<setsize;i++)
    {
        a[i]=LRU[i]+1;
    }
    return a;
}

int LRUmax(int setsize, int *LRU) //find the max LRU
{
    int max=0;
    int LRUmax=0;
    for(int i=0;i<setsize;i++)
    {
        if(max<LRU[i])
        {
            max=LRU[i];
            LRUmax=i;
        }
    }
    return LRUmax;
}

int main(int argc, char* argv[]){


    
    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while(!cache_params.eof())  // read config file
    {
      cache_params>>dummyLine;
      cache_params>>cacheconfig.L1blocksize;
      cache_params>>cacheconfig.L1setsize;              
      cache_params>>cacheconfig.L1size;
      cache_params>>dummyLine;              
      cache_params>>cacheconfig.L2blocksize;           
      cache_params>>cacheconfig.L2setsize;        
      cache_params>>cacheconfig.L2size;
      }
    
  
  
   // Implement by you: 
   // initialize the hirearch cache system with those configs
   // probably you may define a Cache class for L1 and L2, or any data structure you like
    unsigned int i,j;
    int maxLRU;
    Address AddrL1, AddrL2;
    
    //divide address
    AddrL1.Indexbits=log2(cacheconfig.L1size*1024/(cacheconfig.L1setsize*cacheconfig.L1blocksize));
    AddrL1.Offsetbits=log2(cacheconfig.L1blocksize);
    AddrL1.Tagbits=32-AddrL1.Indexbits-AddrL1.Offsetbits;
    AddrL1.Indexsize=pow(2,AddrL1.Indexbits);
    AddrL2.Indexbits=log2(cacheconfig.L2size*1024/(cacheconfig.L2setsize*cacheconfig.L2blocksize));
    AddrL2.Offsetbits=log2(cacheconfig.L2blocksize);
    AddrL2.Tagbits=32-AddrL2.Indexbits-AddrL2.Offsetbits;
    AddrL2.Indexsize=pow(2,AddrL2.Indexbits);
    //create array
    //initialize cache L1,L2, set tag/valid/LRU=0
    unsigned int **L1Tag=new unsigned int *[AddrL1.Indexsize]; //row=#index
    for(i=0;i<AddrL1.Indexsize;i++)
    {
        L1Tag[i]=new unsigned int [cacheconfig.L1setsize]; //column=#way
        for(j=0;j<cacheconfig.L1setsize;j++)
            L1Tag[i][j]=0;
    }
    int **L1Valid=new int *[AddrL1.Indexsize];
    for(i=0;i<AddrL1.Indexsize;i++)
    {
        L1Valid[i]=new int [cacheconfig.L1setsize];
        for(j=0;j<cacheconfig.L1setsize;j++)
            L1Valid[i][j]=0;
    }
    int **L1LRU=new int *[AddrL1.Indexsize];
    for(i=0;i<AddrL1.Indexsize;i++)
    {
        L1LRU[i]=new int [cacheconfig.L1setsize];
        for(j=0;j<cacheconfig.L1setsize;j++)
            L1LRU[i][j]=0;
    }
    unsigned int **L2Tag=new unsigned int *[AddrL2.Indexsize];
    for(i=0;i<AddrL2.Indexsize;i++)
    {
        L2Tag[i]=new unsigned int [cacheconfig.L2setsize];
        for(j=0;j<cacheconfig.L2setsize;j++)
            L2Tag[i][j]=0;
    }
    int **L2Valid=new int *[AddrL2.Indexsize];
    for(i=0;i<AddrL2.Indexsize;i++)
    {
        L2Valid[i]=new int [cacheconfig.L2setsize];
        for(j=0;j<cacheconfig.L2setsize;j++)
            L2Valid[i][j]=0;
    }
    int **L2LRU=new int *[AddrL2.Indexsize];
    for(i=0;i<AddrL2.Indexsize;i++)
    {
        L2LRU[i]=new int [cacheconfig.L2setsize];
        for(j=0;j<cacheconfig.L2setsize;j++)
            L2LRU[i][j]=0;
    }
    
    
  int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
  int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;
   
   
    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";
    
    traces.open(argv[2]);
    tracesout.open(outname.c_str());
    
    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;        
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;
    
    if (traces.is_open()&&tracesout.is_open()){    
        while (getline (traces,line)){   // read mem access file and access Cache
            
            istringstream iss(line); 
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);
            
            //get Tag/Index/Offset from address
            AddrL1.Tag=addr/pow(2,(32-AddrL1.Tagbits));
            AddrL2.Tag=addr/pow(2,(32-AddrL2.Tagbits));
            AddrL1.Index=(addr-AddrL1.Tag*pow(2,(32-AddrL1.Tagbits)))/pow(2,(AddrL1.Offsetbits));
            AddrL2.Index=(addr-AddrL2.Tag*pow(2,(32-AddrL2.Tagbits)))/pow(2,(AddrL2.Offsetbits));
            
            int L1tagflag, L2tagflag;
            
           // access the L1 and L2 Cache according to the trace;
              if (accesstype.compare("R")==0)
              {
                  //Implement by you:
                  // read access to the L1 Cache,
                  //  and then L2 (if required),
                  //  update the L1 and L2 access state variable;
                  L1tagflag=0;
                  L2tagflag=0;
                  for(j=0;j<cacheconfig.L1setsize;j++) //every way
                  {
                      if(L1Tag[AddrL1.Index][j]==AddrL1.Tag) //L1tag=addrtag
                      {
                          L1tagflag=1; //tag fit
                          if(L1Valid[AddrL1.Index][j]) //valid=1
                          {
                              L1AcceState=1; //L1=RH
                              L2AcceState=0; //L2=NA
                              L1LRU[AddrL1.Index]=LRUaugment(cacheconfig.L1setsize, L1LRU[AddrL1.Index]); //LRU+1
                              L1LRU[AddrL1.Index][j]=0; //update this LRU
                              break; //end loop
                          }
                          else //valid=0
                          {
                              L1AcceState=2; //L1=RM
                              L1LRU[AddrL1.Index]=LRUaugment(cacheconfig.L1setsize, L1LRU[AddrL1.Index]);
                              L1LRU[AddrL1.Index][j]=0; //update LRU
                              break;
                          }
                      }
                  } //end for loop
                  if(!L1tagflag) //no tag fit
                  {
                      L1AcceState=2; //L1=RM
                      L1LRU[AddrL1.Index]=LRUaugment(cacheconfig.L1setsize, L1LRU[AddrL1.Index]);
                      maxLRU=LRUmax(cacheconfig.L1setsize, L1LRU[AddrL1.Index]); //find max LRU
                      L1Tag[AddrL1.Index][maxLRU]=AddrL1.Tag; //evict
                      L1Valid[AddrL1.Index][maxLRU]=1;
                      L1LRU[AddrL1.Index][maxLRU]=0;
                      //L1Tag[AddrL1.Index]
                  }
                  if(L1AcceState==2) //L1=RM,then L2 level
                  {
                      for(j=0;j<cacheconfig.L2setsize;j++)
                      {
                          if(L2Tag[AddrL2.Index][j]==AddrL2.Tag) //L2tag=addrtag
                          {
                              L2tagflag=1; //tag fit
                              if(L2Valid[AddrL2.Index][j]) //valid=1
                              {
                                  L2AcceState=1; //L1=RM,L2=RH
                                  L2LRU[AddrL2.Index]=LRUaugment(cacheconfig.L2setsize, L2LRU[AddrL2.Index]);
                                  L2LRU[AddrL2.Index][j]=0;
                                  break;
                              }
                              else //valid=0
                              {
                                  L2AcceState=2; //L1=RM,L2=RM
                                  L2LRU[AddrL2.Index]=LRUaugment(cacheconfig.L2setsize, L2LRU[AddrL2.Index]);
                                  L2LRU[AddrL2.Index][j]=0;
                                  break;
                              }
                          }
                      } //end loop
                      if(!L2tagflag) //no tag fit
                      {
                          L2AcceState=2; //L1=RM,L2=RM
                          L2LRU[AddrL2.Index]=LRUaugment(cacheconfig.L2setsize, L2LRU[AddrL2.Index]);
                          maxLRU=LRUmax(cacheconfig.L2setsize, L2LRU[AddrL2.Index]);
                          L2Tag[AddrL2.Index][maxLRU]=AddrL2.Tag; //evict
                          L2Valid[AddrL2.Index][maxLRU]=1;
                          L2LRU[AddrL2.Index][maxLRU]=0;
                      }
                  }
              }
                 
                 
                 
                 
            
              else
              {
                  //Implement by you:
                  // write access to the L1 Cache,
                  //and then L2 (if required),
                  //update the L1 and L2 access state variable;
                  L1tagflag=0;
                  L2tagflag=0;
                  for(j=0;j<cacheconfig.L1setsize;j++)
                  {
                      if(L1Tag[AddrL1.Index][j]==AddrL1.Tag)
                      {
                          L1tagflag=1; //tag fit
                          if(L1Valid[AddrL1.Index][j]) //valid=1
                          {
                              L1AcceState=3; //L1=WH
                              L1LRU[AddrL1.Index]=LRUaugment(cacheconfig.L1setsize, L1LRU[AddrL1.Index]);
                              L1LRU[AddrL1.Index][j]=0;
                              break;
                          }
                          else //valid=0
                          {
                              L1AcceState=4; //L1=WM
                              break; //no update LRU
                          }
                      }
                  } //end loop
                  if(!L1tagflag) //no tag fit
                  {
                      L1AcceState=4;
                  }
                  for(j=0;j<cacheconfig.L2setsize;j++) //L2 level
                  {
                      if(L2Tag[AddrL2.Index][j]==AddrL2.Tag)
                      {
                          L2tagflag=1; //tag fit
                          if(L2Valid[AddrL2.Index][j]) //valid=1
                          {
                              L2AcceState=3; //L2=WH
                              L2LRU[AddrL2.Index]=LRUaugment(cacheconfig.L2setsize, L2LRU[AddrL2.Index]);
                              L2LRU[AddrL2.Index][j]=0;
                              break;
                          }
                          else //valid=0
                          {
                              L2AcceState=4; //L2=WM
                              break;
                          }
                      }
                  } //end loop
                  if(!L2tagflag) //no tag fit
                  {
                      L2AcceState=4; //L2=WM
                  }
                
              }
            
            tracesout<< L1AcceState << " " << L2AcceState << endl;  // Output hit/miss results for L1 and L2 to the output file;
             
             
        }
        traces.close();
        tracesout.close(); 
    }
    else cout<< "Unable to open trace or traceout file ";
    //delect array
    for(i=0;i<AddrL1.Indexsize;i++)
        delete [] L1Tag[i];
    delete [] L1Tag;
    for(i=0;i<AddrL1.Indexsize;i++)
        delete [] L1Valid[i];
    delete [] L1Valid;
    for(i=0;i<AddrL1.Indexsize;i++)
        delete [] L1LRU[i];
    delete [] L1LRU;
    
    for(i=0;i<AddrL2.Indexsize;i++)
        delete [] L2Tag[i];
    delete [] L2Tag;
    for(i=0;i<AddrL2.Indexsize;i++)
        delete [] L2Valid[i];
    delete [] L2Valid;
    for(i=0;i<AddrL2.Indexsize;i++)
        delete [] L2LRU[i];
    delete [] L2LRU;


   
    
  

   
    return 0;
}
