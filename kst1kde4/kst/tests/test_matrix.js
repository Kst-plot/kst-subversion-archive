//unit test for the Matrix Class

//test Property min
function minTest(){
var result=new Array();
/*case1:default matrix constructor
expected result: matrix.min=0*/
var case1=new Boolean();
var mx=new Matrix();
if(mx.min==0)
case1=true;
else
{case1=false;result.push("Line 10:minTest() case1->Failed\n");}

/*case2:col#>1 and/or row#>1
expected result: matrix.min=the samllest value of matrix entries*/
col=5;
row=4;
var a=new Array();
mx.resize(col,row);
for(var i=0;i<col;i++)
{
for(var j=0;j<row;j++)
{
mx.setValue(j,i,Math.random())
a.push(mx.value(j,i))
}
}
a.sort();
if(a[0]==mx.min)
case2=true;
else
{case2=false;result.push("Line 31:minTest() case2->Failed\n")}
//return result
if(case1&&case2)
result.push("minTest() past\n");
else
result.push("minTest() failed\n");
return result;
}

//test Property max
function maxTest(){
var result=new Array();
/*case1:default constructor
expected result:mx.max=0*/
var case1=false;
var mx=new Matrix();
if(mx.max==0)
case1=true
else
{case1=true;result.push("Line 49:maxTest() case1->Failed\n")}

/*case2:col#>1 and/or row#>1
expected result: matrix.min=the samllest value of matrix entries*/
col=5;
row=4;
var a=new Array();
mx.resize(col,row);
for(var i=0;i<col;i++)
{
for(var j=0;j<row;j++)
{
mx.setValue(j,i,Math.random())
a.push(mx.value(j,i))
}
}
a.sort();
a.reverse();
if(a[0]==mx.max)
case2=true;
else
{case2=false;result.push("Line 70:maxTest() case2->Failed\n")}
//return result
if(case1&&case2)
result.push("maxTest() past\n")
else
result.push("maxTest() failed\n")
return result;
}

//test property mean
function meanTest(){
var result=new Array();
/*case1:default matrix constructor
expected result: matrix.mean=0*/
case1=true;
var mx=new Matrix();
if(mx.mean==0)
case1=true;
else
{case1=false;result.push("Line 89:meanTest() case1->Failed\n")}

/*case2:col#>1 and/or row#>1
expected result: matrix.mean=the average value of matrix entries*/
case2=false;
col=5;
row=4;
var a=new Array();
mx.resize(row,col);
for(var i=0;i<col;i++)
{
for(var j=0;j<row;j++)
{
mx.setValue(i,j,Math.random())
a.push(mx.value(i,j))
}
}
v=new Vector(a);
wait();
if(mx.mean==v.mean)
case2=true;
else
result.push("Line 111:meanTest() case2->Failed\n")

//return result;
if(case1&&case2)
result.push("meanTest() past\n")
else
result.push("meanTest() failed\n")
return result;
}

//test property rows
function rowsTest(){
var result=new Array();
/*case1:default constructor
expected result:mx.rows=1*/
case1=false;
var mx=new Matrix();
if(mx.rows==1)
case1=true;
else
result.push("Line 130:rowsTest() case1 failed\n")

/*case2:row#>=1*/
case2=false
rows=10;
mx.resize(rows,2)
if(mx.rows==rows)
case2=true;
else
result.push("Line 140:rowsTest() case2 failed\n")

//return result
if(case1&&case2)
result.push("rowsTest() past\n")
else
result.push("rowTest() failed\n")
return result;
}

//Test property columns
function columnsTest(){
var result=new Array();
/*case1:default constructor
expected result:mx.columns=1*/
case1=false;
var mx=new Matrix();
if(mx.columns==1)
case1=true
else
result.push("Line 159:columnsTest() case1->faile\n")

/*case2:columns#>=1*/
case2=false;
cols=10;
mx.resize(2,cols)
if(mx.columns==cols)
case2=true;
else
result.push("Line 168:columnTest() case2->failed\n")

if(case1&&case2)
result.push("columnTest() past\n")
else
result.push("columnTest() failed\n")
return result;
}

//Test value(column,row)
function valueTest(){
var result=new Array();
/*case1:default constructor
expected result: m.value(0,0)==0*/
case1=false;
var m=new Matrix();
if(m.value(0,0)==0)
case1=true;
else
result.push("Line 186:valueTest() case1->failed\n")

/*case2:rows#>1 and/or columns#>=1*/
case2=true;
row=3;
col=2;
m.resize(row,col);
var a=new Array();
for(var i=0;i<col;i++)
{
   for(var j=0;j<row;j++)
     {
      s=Math.random(); 
      m.setValue(i,j,s);
      a.push(s);
     }
}

for(var i=0;i<col;i++)
{
   for(var j=0;j<row;j++)
   {
      ind=i*row+j;
    if(m.value(i,j)!=a[ind])
      {
	case2=false;
        result.push("Line 212: valueTest() case2->failed\n");
        break;
      }
   }
}

//return result
if(case1&&case2)
result.push("valueTest() past\n")
else
result.push("valueTest() failed\n")
return result
}

//test setValue(column,row,value)
function setValueTest()
{
var result=new Array();
/*case1: row#>=1 and/or col#>=1*/
case1=false;
row=4;
col=5;
m.resize(row,col);
value=Math.random();
m.setValue(3,2,value);
if(m.value(3,2)==value)
case1=true;
else
result.push("Line 242: setValueTest() case1->failed\n");

//return result
if(case1)
result.push("setValueTest() past\n");
else
result.push("setValueTest() failed\n");
return result;
}row=3;
col=2;
m.resize(row,col);
var a=new Array();
for(var i=0;i<col;i++)
{
   for(var j=0;j<row;j++)
     {
      s=Math.random(); 
      m.setValue(i,j,s);
      a.push(s);
     }
}

//test resize(rows, columns)
function resizeTest(){
var result =new Array();
/*case1:rows>=1 and/or columns>=1*/
case1=false;
var m=new Matrix();
rows=5; cols=4;
m.resize(rows,cols);
if(m.rows==rows&&m.columns==cols)
case1=true;
else
result.push("Line 263: resizeTest() case1->failed\n")

//return result
if(case1)
result.push("resizeTest() past\n");
else
result.push("resizeTest() failed\n");
return result;
}

//test zero()
function zeroTest(){
var result=new Array();
/*case1:rows>=1 and/or columns>=1*/
case1=true;
m=new Matrix();
row=5;
col=3;
m.resize(row,col);
for(var i=0;i<col;i++)
{
   for(var j=0;j<row;j++)
     {
      s=Math.random(); 
      m.setValue(i,j,s);
     }
}
m.zero();
for(var i=0;i<col;i++)
{
   for(var j=0;j<row;j++)
    {
      if(m.value(i,j)!=0)
        {
         case1=false;
         result.push("Line 310: zeroTest() case1->failed\n");
         break;
        }    
    }
if(!case1)
break;
}

//return result
if(case1)
result.push("zeroTest() past\n")
else
result.push("zeroTest() failed\n")
return result;
}
alert(minTest()+maxTest()+meanTest()+rowsTest()+columnsTest()+valueTest()+setValueTest()+resizeTest()+zeroTest());
function wait()
{
var time=0
while(time!=500000)
time++;
}
