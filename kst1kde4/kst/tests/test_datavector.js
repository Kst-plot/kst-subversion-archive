/*unit test for DataVector()
DataVector constructor: DataVector(source,field,start#,#frames to read(n#), skip#, boxcar)
*/

var data_source=new DataSource("/home/vyiwen/graphics/kst/tests/asciimatrix.txt")
var test1=new DataVector(data_source,"test1");
var test2=new DataVector(data_source,"test2");

//Test property skip and skipLength
function skipTest()
{
var result= new Array();

/*case1:test default DataVector constructor
expected result: skip=false and skipLength=0 and vector length=tot.sample#*/
var case1=new Boolean();
var data_v=new DataVector(data_source,"test1");
wait();
if(data_v.skip==false&&data_v.skipLength==0&&data_v.length==999)
case1=true;
else
{case1=false;result.push("Line 16: skipTest() Case1-->Failed\n");}
/*case2:test manully specifying skip#=0
expected result: skip=false and skipLength=0*/
var case2=new Boolean();
var data_v=new DataVector(data_source,"test1",0,10,0,false);
wait();
if(data_v.skip==false&&data_v.skipLength==0&&data_v.length==10)
case2=true;
else
{case2=false;result.push("Line 25: skipTest() Case2-->Failed\n");}

/*case3:test manully specifying #skip =1
expected result: skip=false and skipLength=0 and vector Length=n#*/
var case3=new Boolean();
var data_v=new DataVector(data_source,"test2",0,5,1)
wait();
if(data_v.skip==false&&data_v.skipLength==0&&data_v.length==5)
case3=true;
else
{case3=false;result.push("Line 35: skipTest() Case3-->Failed\n");}

/*case4:test manually specifying skip#= a number other than 0and1
expeceted result: skip=true and skipLength=number specifed and vector Length=(n#/skip#)*/
var case4=new Boolean();
var data_v=new DataVector(data_source,"test1",0,10,2,false);
wait();
if(data_v.skip&&data_v.skipLength==2&&data_v.length==5)
case4=true;
else
{case4=false;result.push("Line 45: skipTest() Case4-->Failed\n");}

//return result
if(case1&&case2&&case3&&case4)
result.push("skipTest() past\n")
else
result.push("skipTest() failed\n")
return result
}

//Test property boxcar
function boxcarTest(){
var result=new Array();
/*case1:specify boxcar to be true
expected result: each vector entry is the average value of all samples in the frame*/
var case1=new Boolean();
var data_v=new DataVector(data_source,"test1",0,10,2,true);
wait();

if(data_v.length==5&&
   data_v[0]==(test1[0]+test1[1])*0.5&&
   data_v[1]==(test1[2]+test1[3])*0.5&&
   data_v[2]==(test1[4]+test1[5])*0.5&&
   data_v[3]==(test1[6]+test1[7])*0.5&&
   data_v[4]==(test1[8]+test1[9])*0.5)
data=true;
else
data=false;
   
if(data_v.boxcar&&data)
case1=true
else
{case1=false;result.push("Line 77: boxcarTest() case1->Failed\n")}

/*case2:specify boxcar to be false
expected result:each vector entry is the first sample value in each frame*/
var case2=new Boolean();
var data_v=new DataVector(data_source,"test2",0,10,2,false);
wait();
if(data_v.length==5&&
   data_v[0]==test2[0]&&
   data_v[1]==test2[2]&&
   data_v[2]==test2[4]&&
   data_v[3]==test2[6]&&
   data_v[4]==test2[8])
data=true;
else
data=false;
if(!data_v.boxcar&&data)
case2=true;
else
{case2=false;result.push("Line 96: boxcarTest() case2->Failed\n")}

/*case3:boxcar value for default constructor should be false*/
var case3=new Boolean();
var data_v=new DataVector(data_source,"test2")
wait();

if(!data_v.boxcar&&data_v.length==999)
case3=true;
else
{case3=false;result.push("Line 106: boxcarTest() case3->Failed\n")}

//return result
if(case1&&case2&&case3)
result.push("boxcarTest() past\n");
else
result.push("boxcarTest() failed\n");
return result;
}

function framesTest(){
var result=new Array();
/*1<#n<tot.sample #(999)*/
/*case1: #n%#skip!=0*/
//expected result:frames read from source=(#n/#skip)*#skip
var case1=new Boolean()
var v=new DataVector(data_source,"test1",0,20,3)
wait();
if(v.frames==18)//(20/3)=6 frames read=6*3=18
case1=true;
else
{case1=false;result.push("Line 127: frameTest() case1->Failed\n")}

/*case2: #n%#skip=0*/
//expected result:frames read from source=#n
var case2=new Boolean()
var v=new DataVector(data_source,"test1",0,99,3)
wait();
if(v.frames==99)
case2=true;
else
{case1=false;result.push("Line 138: frameTest() case2->Failed\n")}

/*case3:#n=1
expecet result: DataVector.frames=2*/
var case3=new Boolean()
var v=new DataVector(data_source,"test1",0,1)
wait();
if(v.frames==2)
case3=true
else
{case3=false;result.push("Line 147: frameTest() case3->Failed\n")}

/*#n>tot.sample#(999)*/
/*case4:tot.sample#%skip#=0
expected result: DataVector.frames=tot# samples*/
var case4=new Boolean()
var v=new DataVector(data_source,"test1",0,1000,9)//999%9=0
wait();
if(v.frames==999)
case4=true
else
{case4=false; result.push("Line 158: frameTest() case4->Failed\n")}

/*case5:tot# sample/skip#!=0
expected result:DataVector.frames=#n-#skip, #skip!=0 and 1*/
var case5=new Boolean()
var v=new DataVector(data_source,"test1",0,1000,10)//999%10=9
wait();
if(v.frames=990)
case5=true;
else
{case5=false;result.push("Line 168: frameTest() case5->Failed\n")}

//return result
if(case1&&case2&&case3&&case4&&case5)
result.push("framesTest() past\n")
else
result.push("framesTest() failed\n")
return result;
}

/*Test Property framesRequested*/
function framesRequestedTest()
{
var result=new Array();
/*case1:default constructor
expected result: dataVector.framesRequested=-1*/
var case1=true;
var v=new DataVector(data_source,"test1")
if(v.framesRequested==-1)
case1=true;
else
{case1=false;result.push("Line 189: framesRequestedTest() case1->Failed\n")}

/*case2:#n=1
expected result: dataVector.frameRequested=2*/
var case2=true;
var v=new DataVector(data_source,"test1",0,1)
wait()
if(v.framesRequested==2)
case2=true;
else
{case2=false;result.push("Line 199: framesRequestedTest() case2->Failed\n")}

/*case3:#n!=1
expected result: dataVector.frameRequested=#n*/
var case3=true;
var v=new DataVector(data_source,"test1",0,1000)
wait();
if(v.framesRequested==1000)
case3=true;
else
{case3=false;result.push("Line 209: frameRequestedTest() case3->Failed\n")}

//return result
if(case1&&case2&&case3)
result.push("framesRequestedTest() past\n")
else
result.push("frameRequestedTest() failed\n")
return result;
}

//Test startFrame
function startFrameTest()
{
var result=new Array()
/*case1:default constructor
expected result: dataVector.startFrame=0*/
var case1=new Boolean();
var v=new DataVector(data_source,"test2")
if(v.startFrame==0)
case1=true;
else
{case2=false;result.push("Line 230: startFrameTest() case1->Failed\n")}

/*case2:0<=start#<=tot.sample#-1
expected result: dataVector.startFrame=start#*/
var case2=new Boolean();
var v=new DataVector(data_source,"test2",25,10);
wait();
if(v.startFrame==25&&v[0]==test2[25])
case2=true;
else
{case2=false;result.push("Line 240:startFrameTest() case2->Failed\n");}

/*case3: start#<0
expected result:dataVector.startFrame=tot.sample#-#n*/
var case3=new Boolean();
var v=new DataVector(data_source,"test2",-1,11)
wait();
start=999-11;
if(v.startFrame==start&&v[0]==test2[start])
case3=true;
else
{case3=false;result.push("Line 262:startFrameTest() case3->Failed\n");}

//return result
if(case1&&case2&&case3)
result.push("startFrameTest() past\n")
else
result.push("startFrameTest() failed\n")
return result;
}

/*Test Property: startFrameRequested*/
function startFrameRequestedTest(){
var result=new Array();
/*case1: default constructor
expected result:dataVector.startFrameRequested=0*/
var case1=new Boolean();
var v=new DataVector(data_source,"test2");
if(v.startframeRequested==0)
case1=true;
else
{case1=false; result.push("Line 271: startFrameTest() case1->Failed\n")}

/*case2:start# specified*/
var case2=new Boolean();
var v=new DataVector(data_source,"test2",900,10)
wait();
if(v.startFrameRequested==900)
case2=true;
else
{case2=false;result.push("Line 280:startFrameTest() case2->Failed\n")}
//return result
if(case1&&case2)
result.push("startFrameTest() past\n")
else
result.push("startFrameTest() failed\n")
return result
}

/*Test Property:readToEnd*/
function readToEndTest(){
var result=new Array();
/*case1:default constructor
expected result:dataVector.readToEnd=true*/
var case1=new Boolean();
var v=new DataVector(data_source,"test1")
if(v.readToEnd)
case1=true
else
{case1=false;result.push("Line 299:readToEndTest() case1->Failed\n")}

/*case2:#n<=0
expected result:dataVector.readToEnd=true*/
var case2=new Boolean();
var v=new DataVector(data_source,"test1",988,-1)
wait()
if(v.readToEnd)
case2=true;
else
{case2=false; result.push("Line 309:readToEndTest() case2->Failed\n")}

/*case3: #n>0
expected result:dataVector.readToEnd=false*/
var case3=new Boolean();
var v=new Boolean();
var v=new DataVector(data_source,"test1",988,10)
wait()
if(!v.readToEnd)
case3=true;
else
{case3=false;result.push("Line 320:readToEndTest() case3->Failed\n")}

//return result
if(case1&&case2&&case3)
result.push("readToEndTest() past\n")
else
result.push("readToEndTest() faile\n")
return result;
}

/*Test property:countFromEnd*/
function countFromEndTest(){
var result=new Array()
/*case1:default constructor
expected result:dataVector.countFromEnd=false*/
var case1=new Boolean();
var v=new DataVector(data_source,"test1")
if(!v.countFromEnd)
case1=true
else
{case1=false;result.push("Line 343:countFromEndTest() case1->Failed\n")}

/*case2:start#>=0
expected result:dataVector.countFromEnd=false*/
var case2=new Boolean();
var v=new DataVector(data_source,"test1",10,10)
wait();
if(!v.countFromEnd)
case2=true
else
{case2=false;result.push("Line 352:countFromEndTest() case2->Failed\n")}

/*case3:start#<0
expected result:dataVector.countFromEnd=true*/
var case3=new Boolean();
var v=new DataVector(data_source,"test1",-1,10);
wait();
if(v.countFromEnd)
case3=true
else
{case3=false;result.push("Line 361:countFromEndTest() case3->Failed\n")}
//return result
if(case1&&case2&&case3)
result.push("countFromEndTest() past\n")
else
result.push("countFromEndTest() failed\n")
return result;
}

/*Test field*/
function fieldTest()
{
var result=new Array()
/*case1: return the name of the field for test1*/
if(test1.field=="test1")
case1=true;
else
{case1=false;result.push("Line 380: fieldTest() case1-->Failed\n")}

/*case2:return the name of the field for test2*/
if(test2.field="test2")
case2=true;
else
{case2=false;result.push("Line 386: fieldTest() case2-->Failed\n")}

//return result
if(case1&&case2)
result.push("fieldTest() past\n")
else
result.push("fieldTest() failed\n")
return result;
}
//test propety datasource
function dataSourceTest(){
var result=new Array();
//case1:ASCII datasource
if(test1.dataSource.fileName=="/home/vyiwen/graphics/kst/tests/asciimatrix.txt")
case1=true
else
{case1=false;result.push("Line 402:dataSourceTest() case1->Failed\n")}

//return result
if(case1)
result.push("dataSourceTest() past\n");
else
result.push("dataSourceTest() failed\n");
return result;
}

//test changeFrames(start,count[,skip[,ave]])
function changeFramesTest(){
var result=new Array()
/*case1:change the frames of dataVector test1*/
var case1=true;
test1.changeFrames(1,10,2,true);
wait();
var vec1=new DataVector(data_source,"test1",1,10,2,true)
wait();
for(var i=0; i<vec1.length; i++)
{
      if(test1[i]!=vec1[i])
	{
	  case1=false;
	  result.push("Line 426:changeFramesTest() case1->Failed\n")	
	  break;
	}
}
//return result
if(case1)
result.push("changeFramesTest() past\n");
else
result.push("changeFramesTest() failed\n");
return result;
}

//test changeFile(source)
function changeFileTest(){
var result=new Array()
/*case1:switch the source of test1 to another ascii source file*/
var case1=new Boolean();
var url="http://websvn.kde.org/*checkout*/branches/extragear/kde3/graphics/kst/tests/data_files/fittest.dat?revision=799586&content-type=text%2Fplain";
var s=new DataSource(url)
test1.changeFile(s);
if(test1.dataSource.fileName==url)
case1=true;
else
{case1=false;result.push("Line 449: changeFileTest()->Failed")}

//return result
if(case1)
result.push("changeFile(source) Test past\n")
else
result.push("changeFile(source) Test failed\n")
return result
}
alert("Test Results:\n"+skipTest()+boxcarTest()+framesTest()+framesRequestedTest()+startFrameTest()+readToEndTest()+countFromEndTest()+fieldTest()+dataSourceTest()+changeFramesTest()+changeFileTest());
function wait()
{
var time=0
while(time!=500000)
time++;
}
