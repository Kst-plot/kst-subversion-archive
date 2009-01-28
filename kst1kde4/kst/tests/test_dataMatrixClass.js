//unit test for the DataMatrix Class in KstScript
var s=new DataSource("/home/vyiwen/graphics/kst/tests/asciimatrix.txt")
field="[MATRIX,100,0,0,1,1]"
mx=new DataMatrix(s,field);

//Test property skip and skipLength
function skipTest()
{
var result=new Array();
/*case1:default constructor-->m.skip=true and m.skipLength<=0*/
case1=false;
if(!mx.skip&&mx.skipLength<=0)
case1=true;
else
result.push("Line 11:skipTest() case1->failed\n")

/*case2:skip#>1-->m.skip=true and m.skipLength=skip#*/
case2=false;
skip=2;
var m=new DataMatrix(s,field,0,0,10,10,skip)
wait();
if(m.skip&&m.skipLength==skip&&compareSkip(mx,m,2,10,10))
case2=true;
else
result.push("Line 23: skipTest() case2->failed\n")

/*case3:skip#<=1-->m.skip=false and m.skipLength<=1*/
case3=false;
skip=-1
var m=new DataMatrix(s,field,0,0,10,10,skip)
wait();
if(!m.skip&&m.skipLength<=1)
case3=true;
else
result.push("Line 31: skipTest() case3->failed\n")
//return result
if(case1&&case2&&case3)
result.push("skipTest() past\n")
else
result.push("skipTest() failed\n")
return result
}

//test property boxcar
function boxcarTest(){
var result=new Array();
/*case1:default constructor*/
case1=false;
if(!mx.boxcar)
case1=true;
else
result.push("boxcarTest() case1->failed\n")

/*case2: set ave=true in constructor*/
skip=2;
var m=new DataMatrix(s,field,0,0,5,5,skip,true)
wait();
case2=false;
if(m.boxcar&&compareBoxcar(mx,m,skip,5,5))
case2=true;
else
result.push("Line 58: boxcarTest() case2->failed\n")

/*case3: set ave=false in constructor*/
case3=false;
var x=new DataMatrix(s,field,0,0,5,5,skip,false);
wait();
if(!x.boxcar&&!compareBoxcar(mx,x,skip,5,5))
case3=true
else
result.push("Line 68: boxcarTest() case3->failed\n")

//return result
if(case1&&case2&&case3)
result.push("boxcarTest() past\n")
else
result.push("boxcarTest() failed\n")
return result;
}

//test Property: xReadToEnd
function xReadToEndTest(){
var result=new Array();
/*case1: default constructor: xReadToEnd=true*/
case1=false;
if(mx.xReadToEnd)
case1=true;
else
result.push("Line 86:xReadToEndTest() case1 failed\n")

/*case2: xN>0: xReadToEnd=false*/
case2=false;
xN=10;
var m=new DataMatrix(s,field,0,0,xN,10)
wait()
if(!m.xReadToEnd)
case2=true;
else
result.push("Line 96:xReadToEndTest() case2 failed\n")

/*case3: xN<=0: xReadToEnd=true*/
case3=false;
xN=-1
var m=new DataMatrix(s,field,0,0,xN,10)
wait()
if(m.xReadToEnd&&m.columns==mx.columns)
case3=true;
else
result.push("Line 106:xReadToEndTest() case3 failed\n")
//return result
if(case1&&case2&&case3)
result.push("xReadToEndTest() past\n")
else
result.push("xReadToEndTest() failed\n")
return result;
}

//test property:yReadToEnd
function yReadToEndTest(){
var result=new Array();
/*case1:default constructor:yReadToEnd=true*/
case1=false;
if(mx.yReadToEnd)
case1=true;
else
result.push("Line 123:yReadToEndTest() case1 failed\n")

/*case2:yN>0: yReadToEnd=false*/
case2=false;
yN=10;
var m=new DataMatrix(s,field,0,0,5,yN)
if(!m.yReadToEnd)
case2=true;
else
result.push("Line 132:yReadToEndTest() case2 failed\n")

/*case3:yN<0: yReadToEnd=true*/
case3=false;
yN=-1;
var m=new DataMatrix(s,field,0,0,5,yN)
wait();
if(m.yReadToEnd&&m.rows==mx.rows)
case3=true;
else
result.push("Line 141:yReadToEndTest() case3 failed\n")

//return result
if(case1&&case2&&case3)
result.push("yReadToEndTest() past\n")
else
result.push("yReadToEndTest() failed\n")
return result;
}

//test Property xCountFromEnd
function xCountFromEndTest(){
var result=new Array();
/*case1:default constructor->xCountFromEnd=false*/
case1=false;
if(!mx.xCountFromEnd)
case1=true;
else
result.push("Line 160:xCountFromEndTest() case1 failed\n")

/*case2:xStart<0->xCountFromEnd=true*/
case2=false;
xStart=-1;
m1=new DataMatrix(s,field,xStart,0,5,5)
wait();
m2=exMx(xStart,0,5,5)
wait();

if(m1.xCountFromEnd&&sameMx(m1,m2))
case2=true
else
result.push("Line 171: xCountFromEndTest() case2 failed\n")

/*case3:xStart>=0->xCountFromEnd=true*/
case3=false;
xStart=10;
m=new DataMatrix(s,field,xStart,0,5,5)
wait();
if(!m.xCountFromEnd)
case3=true
else
result.push("Line 183: xCountFromEndTest() case3 failed\n")
//return result
if(case1&&case2&&case3)
result.push("xCountFromEndTest() past\n")
else
result.push("xCountFromEndTest() failed\n")
return result
}

//Test Property yCountFromEnd
function yCountFromEndTest(){
var result=new Array();
/*case1:default constructor->yCountFromEnd=false*/
case1=false;
if(!mx.yCountFromEnd)
case1=true;
else
result.push("Line 200:yCountFromEndTest() case1 failed\n");
/*case2:yStart<0->yCountFromEnd=true*/
case2=false;
yStart=-1;
var m1=new DataMatrix(s,field,0,yStart,5,5);
wait();
var m2=exMx(0,yStart,5,5);
wait();
if(m1.yCountFromEnd&&sameMx(m1,m2))
case2=true;
else
result.push("Line 211:yCountFromEndTest() case2 failed\n")
/*case3:yStart>=0->yCountFromEnd=false*/
case3=false;
yStart=1;
m=new DataMatrix(s,field,0,yStart,5,5);
wait();
if(!m.yCountFromEnd)
case3=true;
else
result.push("Line 220:yCountFromEndTest() case3 failed\n")
//return result
if(case1&&case2)
result.push("yCountFromEndTest() past\n")
else
result.push("yCountFromEndTest() failed\n")
return result;
}

//Test Property field
function fieldTest(){
var result=new Array();
/*case1:load from field: [MATRIX,100,0,0,1,1]*/
case1=false;
if(mx.field==field)
case1=true;
else
result.push("Line 237: fieldTest() case1 failed\n")

//return result
if(case1)
result.push("fieldTest() past\n")
else
result.push("fieldTest() failed\n")
return result;
}

//Test Property dataSource
function dataSourceTest(){
var result=new Array();
/*case1:load from ASCII source:/home/vyiwen/graphics/kst/tests/asciimatrix.txt*/
case1=false;
if(mx.dataSource.fileName=="/home/vyiwen/graphics/kst/tests/asciimatrix.txt")
case1=true;
else
result.push("Line 255: dataSourceTest() case1 failed\n")

//return result
if(case1)
result.push("dataSourceTest() past\n")
else
result.push("dataSourceTest() failed\n");
return result;
}

//Test method change(xStart,yStart,xCount,yCount[,number skip[,boolean ave]])
function changeTest(){
var result=new Array();
/*case1:apply change(xStart,yStart,xCount,yCount) to mx*/
case1=false;
xS=5;
yS=-1;
xC=-1;
yC=10;
mx.change(xS,yS,xC,yC);
wait();
m=new DataMatrix(s,field,xS,yS,xC,yC)
wait();
if(sameMx(mx,m))
case1=true
else
result.push("Line 281:changeTest() case1 failed\n")

/*case2:apply change(xStar,yStar,xCount,yCount,skip,ave) with skip<=1 to mx*/
case2=false
xS=-1;yS=20;xC=10;yC=10;skip=0; ave=false;
mx.change(xS,yS,xC,yC,skip,ave);
wait();
m=new DataMatrix(s,field,xS,yS,xC,yC,skip,ave);
wait();
if(sameMx(mx,m))
case2=true;
else
result.push("Line 293:changeTest() case2 failed\n")

/*case3:apply change(xStar,yStar,xCount,yCount,skip,ave) with skip>1 to mx*/
case3=false;
skip=2;
mx.change(xS,yS,xC,yC,skip,ave);
wait();
m=new DataMatrix(s,field,xS,yS,xC,yC,skip,ave);
wait();
if(sameMx(mx,m))
case3=true
else
result.push("Line 305: changeTest() case3->failed\n")

//return result
if(case1&&case2&&case3)
result.push("changeTest() past\n")
else
result.push("changeTest() failed\n")
return result;
}

//test changeFile(source)
function changeFileTest(){
var result=new Array();
/*case1:swith to another ascii matrix source*/
case1=false;
field1="/home/vyiwen/graphics/kst/tests/data_files/world.txt"
var s1=new DataSource(field1)
mx.changeFile(s1);
if(mx.dataSource.fileName==field1)
case1=true;
else
result.push("Line 326:changeFileTest() case1->failed\n")

//return result
if(case1)
result.push("changeFileTest() past\n")
else
result.push("changeFileTest() failed\n")
return result;
}
alert(skipTest()+boxcarTest()+xReadToEndTest()+yReadToEndTest()+xCountFromEndTest()+yCountFromEndTest()+fieldTest()+dataSourceTest()+changeTest()+changeFileTest())



//help functions
//wait for a while to let the matrix update
function wait()
{
var time=0
while(time!=500000)
time++;
}

//to test if the matrix specified by a skip# have the expected entries
function compareSkip(m1,m2,skip,xN,yN)
{
var a1=new Array();
var a2=new Array();
for(var i=0;i<xN;i=i+skip)
{
    for(var j=0;j<yN;j=j+skip)
     {
       a1.push(m1.value(i,j));  
     }
}

for(var i=0;i<m2.columns;i++)
{
    for(var j=0;j<m2.rows;j++)
     {
       a2.push(m2.value(i,j));  
     }
}

result=true;
for(var i=0;i<a2.length;i++)
{
if(a1[i]!=a2[i])
{
result=false;
break;
}
}
return result;
}

//to test if the matrix specified by boxcar=true would have the expected entries
function compareBoxcar(m1,m2,skip,xN,yN)
{

var a1=new Array();
var a2=new Array();
for(var i=0;i<xN-1;i=i+skip)
{
    for(var j=0;j<yN-1;j=j+skip)
     {
       square=new DataMatrix(s,field,i,j,skip,skip);
       a1.push(square.mean);  
     }
}

for(var i=0;i<m2.columns;i++)
{
    for(var j=0;j<m2.rows;j++)
     {
       a2.push(m2.value(i,j));  
     }
}

result=true;
for(var i=0;i<a2.length;i++)
{
if(a1[i]!=a2[i])
{
result=false;
break;
}
}
return result;
}

//to extract a portion of matrix
//xN<mx.columns and yN<mx.rows
function exMx(xs,ys,xN,yN)
{
C=mx.columns
R=mx.rows;
if(xs<0&&ys>=0)
{
n_xs=C-xN
ex=new DataMatrix(s,field,n_xs,ys,-1,yN)
}
else if(ys<0&&xs>=0)
{
n_ys=R-yN
ex=new DataMatrix(s,field,xs,n_ys,xN,-1)
}
else
{
n_xs=C-xN;
n_ys=R-yN;
ex=new DataMatrix(s,field,n_xs,n_ys,-1,-1)
}   
return ex;

}


//compare if the two matrices have the same entries
function sameMx(m1,m2)
{
c=m1.columns
r=m1.rows
result=true;
for(var i=0;i<c;i++)
{
  for(var j=0;j<r;j++)
  {
    if(m1.value(i,j)!=m2.value(i,j))
     {
       result=false
        break;
     }
  }
   if(!result)
    break; 
}
return result;
}

