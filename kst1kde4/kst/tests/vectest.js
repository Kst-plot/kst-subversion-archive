//Unit test for the Vector Class of KstScript

//test Properties of Vector Class
//test Property length
function lengthTest()
{
var result = new Array();
//case1:test the length of a new Kst vector constructed by Vector()
//expected result: Vector.length = 1
//set boolean to indicate case1 result
var case1 = new Boolean();
var v = new Vector();
if(v.length==1)
case1 = true;
else
{case1=false;result.push("Line 13: lengthTest() Test Case 1 -->Failed\n");}

//case2:test the length of a new Kst vector constructed by Vector(Array)
//expected result: Vector.length = Array.length
//set boolean to indicate case2 result
var case2=new Boolean();
var arr = new Array();
arr.length = 10;
var v = new Vector(arr);
if(v.length==arr.length)
case2 = true;
else
{case2=false;result.push("Line 25: lengthTest() Test Case 2 -->Failed\n")}; 

//case3: test after copying one kst vector to another, the two vector should have same length
//expected result: Vector1.length = Vector2.length
//set boolean to indicate case3 result
var case3 = new Boolean();
var a1 = new Array(); a1.length=10;
var v1= new Vector(a1);
var v2 = new Vector();
v2 = v1;
if(v1.length==v2.length)
case3=true;
else
{case3=false; result.push("Line 38: lengthTest() Test Case 3 -->Failed\n");}

//return test result for lengthTest() 
if(case1&&case2&&case3)
result.push("length Test Past\n");
else
result.push("length Test Failed\n");
return result;
}

//test Property min
function minTest()
{
var result = new Array();
//case1:test the min value of a Kst Vector initialized with different values is the samllest entry value
//procedures: construct a vector and an array, and fill them with same entries so that they are identical to each other.
//sort the array in ascending order
//expected result: Array[1]==Vector.min
//set boolean to indicate case1 result
var case1=new Boolean();
var arr=new Array(); arr.length=10;
var v=new Vector(arr);
for(var i=0; i<v.length; i++){v[i]=Math.random(); arr.push(v[i]);}
arr.sort();
if(arr[0]==v.min)
case1=true;
else
{case1=false;result.push("Line 65: minTest() Test Case 1 -->Failed\n");}

//case2:test the min value of a empty Kst Vector is NaN
//procedures: construct a new Kst Vector by Vector(), find its min value
//expected result: Vector.min is NaN
//set boolean to indicate case2 result
var case2=new Boolean();
var v=new Vector();
if(isNaN(v.min))
case2=true;
else
{case2=false;result.push("Line 76: minTest() Test Case 2 -->Failed\n");}
//return test result for minTest()
if(case1&&case2)
result.push("min Test Past\n");
else
result.push("min Test Failed\n");
return result;
}
 
//test Property max
function maxTest()
{
var result =new Array();
//case1:test the max value of a Kst Vector initialized with different values is the greatest entry value
//procedures: construct a vector and an array, and fill them with same entries so that they are identical to each other.
//sort the array in descending order
//expected result: Array[1]==Vector.max
//set boolean to indicate case1 result
var case1=new Boolean();
var arr=new Array(); arr.length=10;
var v=new Vector(arr);
arr.length=0;
for(var i=0; i<v.length; i++){v[i]=Math.random(); arr.push(v[i]);}
arr.sort();
arr.reverse();
if(arr[0]==v.max)
case1=true;
else
{case1=false;result.push("Line 104: maxTest() Test Case 1 -->Failed\n");}

/*case2:test the max value of a empty Kst Vector is NaN
procedures: construct a new Kst Vector by Vector(), find its max value
expected result: Vector.max is NaN
set boolean to indicate case2 result*/
var case2=new Boolean();
var v=new Vector();
if(isNaN(v.max))
case2=true;
else
{case2=false; result.push("Line 115: maxTest() Test Case 2 -->Failed\n")}
//return test result for maxTest()
if(case1&&case2)
{result.push("max Test Past\n");}
else
{result.push("max Test Failed\n");}
return result;
}

/*test Property mean*/
function meanTest()
{
var result=new Array();
/*case1:test the mean value of a Kst vector is (sum of all its entries)/length
expected result: Vector.mean=(sum)/(Vector.length) */
//set boolean to indicate case1 result
var case1=new Boolean();
var arr=new Array(); arr.length=10;
var v=new Vector(arr);
var sum=0;
for(var i=0; i<v.length; i++){v[i]=Math.random();sum=sum+v[i];}
if(v.mean==sum/v.length)
case1=true;
else
{case1=false;result.push("Line 139: meanTest() Test Case 1 -->Failed\n")}

/*case2:test the mean value of an empty Kst vector is NaN
expected result: Vector.mean is NaN*/
//set boolean to indicate case2 result
var case2=new Boolean();
var v=new Vector();
if(isNaN(v.mean))
case2=true;
else
{case2=false; result.push("Line 149: meanTest() Test Case 2 -->Failed\n")}

//return test result for meanTest()
if(case1&&case2)
result.push("mean Test Past\n");
else
result.push("mean Test Failed\n");
return result;
}

/*test Property numNaN*/
function numNaNTest()
{
var result = new Array();
/*case1: test numNaN gives the number of NaN in a Kst Vector
expected result: Vector.numNaN = # NaN*/
//set boolean to indicate case1 result
var case1 =new Boolean();
var arr=new Array();
arr.length=5;
v=new Vector(arr)
v[0]=1; v[1]=2;//initialize 2 entries as numbers, and the other 3 are NaN
if(v.numNaN==3)
case1=true;
else
{case1=false;result.push("Line 174: numNaN Test Case 1 -->Failed\n")}

/*case2: test numNaN for an empty Kst Vector equals the vector length
expected result: Vector.numNaN=Vector.length*/
var case2=new Boolean();
var v=new Vector();
if(v.numNaN==v.length)
case2=true;
else
{case2=false; result.push("Line 183: numNaN Test Case 2 -->Failed\n")}
//return test result for numNaNTest()
if(case1&&case2)
result.push("numNaN Test Past\n");
else
result.push("numNaN Test Failed\n");
return result;
}

/*test Property array*/
function arrayTest()
{
var result=new Array();
/*case1: intialize a kst vector with 10 samples and an array using the same entries
Expected result: Vector.array and Array have same entries*/
//set boolean to indicate case1 result
var case1=new Boolean(true);
var arr =new Array(); arr.length=10;
v=new Vector(arr);
for(var i=0; i<v.length; i++){v[i]=Math.random();arr[i]=v[i];}
for(var i=0; i<v.length; i++)
{
	if(v.array[i]!=arr[i])
	{case1=false;
	result.push("Line 209: array Test Case 1 -->Failed\n");	 
	break;}
}

//return test result for arrayTest()
if(case1)
result.push("array Test Past\n");
else
result.push("array Test Failed\n");
return result;
}

//test Methods of the Vector Class

/*test Method resize(number size), size>=2*/
function resizeTest()
{
var result=new Array()
/*case1: resize a Kst Vector to 10
Expected result: Vector.length=10*/
//set boolean to indicate case1 result
var case1=new Boolean();
var v=new Vector();
if(v.editable)
v.resize(10);
if(v.length==10)
case1=true;
else
{case1=false;result.push("Line 236: resize(size) Test Case 1 -->Failed\n")}

/*case2: resize a Kst Vector to 99999*/
/*Expected result: Vector.length=99999*/
//set boolean to indicate case2 result
var case2=new Boolean();
if(v.editable)
v.resize(99999);
if(v.length==99999)
case2=true;
else
{case2=false; result.push("Line 247: resize(size) Test Case 2 -->Failed\n")}

/*case3: resize a Kst Vector to 100*/
/*Expected result: Vector.length=100*/
//set boolean to indicate case3 result
var case3=new Boolean();
if(v.editable)
v.resize(100);
if(v.length==100)
case3=true;
else
{case3=false; result.push("Line 258: resize(size) Test Case 2 -->Failed\n")}

//return test result
if(case1&&case2)
result.push("resize(size) Test Past\n");
else
result.push("resize(size) Test Failed\n")
return result;
}

/*test Method interpolate(i,n)*/
function interplTest()
{
var result=new Array();
/*case1: interpolate the Kst Vector: 1,2,3 to 5(odd) numbers and return the interpolated values of sample number 0,1,2,3,4,5*/
/*expected result: Vector.interpolate(0,5)=1
		   Vector.interpolate(1,5)=1.5
		   Vector.interpolate(2,5)=2
		   Vector.interpolate(3,5)=2.5
		   Vector.interpolate(4,5)=3*/
//set boolean to indicate case1 result
var case1=new Boolean();
var arr=new Array();
arr.push(1);arr.push(2); arr.push(3);
var v=new Vector(arr);//v is a vector with entries: 1,2,3
if(v.interpolate(0,5)==1&&v.interpolate(1,5)==1.5&&v.interpolate(2,5)==2&&v.interpolate(3,5)==2.5&&v.interpolate(4,5)==3)
case1=true;
else
{case1=false;result.push("Line 286: interpolate(i,n) Test  Case 1 -->Failed\n")}

/*case2: interpolate the Kst Vector: 1,2,3 to 6(even) number and return the interpolated values of sample number 0,1,2,3,4,5,6*/
/*expected result: Vector.interpolate(0,6)=1
		   Vector.interpolate(1,6)=1.4
		   Vector.interpolate(2,6)=1.8
		   Vector.interpolate(3,6)=2.2
		   Vector.interpolate(4,6)=2.6
	     	   Vector.interpolate(5,6)=3*/
//set boolean to indicate case2 result
var case2=new Boolean();
if(v.interpolate(0,6)==1&&v.interpolate(1,6)==1.4&&v.interpolate(2,6)==1.8&&v.interpolate(3,6)==2.2&&v.interpolate(4,6)==2.6&&v.interpolate(5,6)==3)
case2=true;
else
{case2=false; result.push("Line 300: interpolate(i,n) Test Case 2 -->Failed\n")}
//return result
if(case1&&case2)
result.push("interpolate(i,n) Test Past\n")
else
result.push("interpolate(i,n) Test Failed\n")
return result;
}

/*test Method zero()*/
function zeroTest(){
var result=new Array();
/*case1: sets all values in a Kst Vector with 10 non-zero entries to zero
expected result: all entries =0*/
//set boolean to indicate case1 result
try{
var case1=new Boolean(true);
var v=new Vector();
v.resize(10);
v[3]=3; v[7]=7;
v.zero();
for(var i=0; i<v.length; i++)
{
if(v[i]!=0)
{case1=false;result.push("Line 323: zeroTest() Test Case 1 -->Failed\n"); break;}
}
}
catch(GeneralError)
{
alert("The Vector is not editable")
}
/*case2: set all values in a Kst Vector with 1000 non zero entries to zero
expected result: all entries=0*/
//set boolean to indicate case2 result
try{
var case2=new Boolean(true);
var v=new Vector();
var v=new Vector();
v.resize(1000);
for(var i=0; i<v.length;i++){v[i]=Math.random();}//initialize vectors with random numbers
v.zero();
for(var i=0;i<v.length;i++)
{
if(v[i]!=0)
{
case2=false;
result.push("Line 345: zeroTest() Test Case 2 -->Faile\n");
break;
}
}
}catch(GeneralError)
{
alert("The Vector is not editable")
}
//return result
if(case1&&case2)
result.push("zeroTest() Test Past\n");
else
result.push("zeroTest() Test Failed\n");
return result;
}

alert(lengthTest()+minTest()+maxTest()+meanTest()+numNaNTest()+arrayTest()+resizeTest()+interplTest()+zeroTest());
