//unit test for the String Class

function valueTest(){
var result=new Array();
//case1: the value of a newly created Kst String is 0
var case1=new Boolean();
var s=new String();
if(s.value=="")
case1=true;
else
{case1=false;result.push("Line 8:valueTest() Test Case1-->Failed\n")}

//case2: modify the value of a Kst String
var case2=new Boolean();
s.value="kst";
if(s.value=="kst")
case2=true
else
{case2=false; result.push("Line 16:valueTest() Test Case2-->Faile\n")}

//return result
if(case1&&case2)
result.push("valueTest() Past\n")
else
result.push("valueTest() Failed\n")
return result
}

alert(valueTest())
