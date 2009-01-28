/*Unit test for the Scalar Class*/

function valueTest()
{
result=new Array();
//case1: a new scalar created with default constructor has a 0 value
var case1=new Boolean()
var s=new Scalar();
if(s.value==0)
case1=true;
else
{case1=false;result.push("Line 9: valueTest() Test Case1-->Failed\n")}
//case2: access the value of a scalar
var case2=new Boolean();
var n=Math.random();
var s=new Scalar(n);
if(s.value==n)
case2=true
else
{case2=false; result.push("Line 16: valueTest() Test Case2-->Failed\n")}

//case3: modify a value
var case3=new Boolean()
n=Math.random();
s.value=n;
if(s.value==n)
case3=true;
else
{case3=false; result.push("Line 26: valueTest() Test Case3-->Failed\n")}

//return reuslt
if(case1&&case2&&case3)
result.push("valueTest() Past\n")
else
result.push("valueTest() Failed\n")
return result
}
alert(valueTest())
