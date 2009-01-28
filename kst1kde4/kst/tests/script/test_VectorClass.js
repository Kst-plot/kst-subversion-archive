/*unit test for Vector the Class*/


err=0;
function assert(x) {
xrc=false;
	try {
		var xrc = eval(x);
                if(!xrc){
                err++;	
                 } 
		
	} catch (e) {
		alert("Error: " + e.name + "\nLast test was: " + x);
		err++;
	}
        finally{
               return xrc;
                }

}

function assertNoReturn(x){
correct=true;
	try{
		eval(x);
         } catch (e) {
		alert("Error: " + e.name + "\nLast test was: " + x);
		err++;
		correct=false;
         }
         finally
            {
              return correct;
            }

}

//test constructor Vector()
assertNoReturn("v1=new Vector()");


//test constructor Vector(array)
var a=new Array();
for(var i=0;i<11;i++){a.push(Math.random());}
assertNoReturn("v2=new Vector(a)")

//test Property: editable
assert("v1.editable");
assert("v2.editable");

//test Property: length
assert("v1.length==1");
assert("v2.length==11");

//test Property: min
assert("isNaN(v1.min)")
a.sort();
assert("v2.min==a[0]")

//test Property: max
assert("isNaN(v1.max)")
a.reverse();
assert("v2.max==a[0]");

//test Property: mean
sum=0;
for(var i=0;i<v2.length;i++){sum=sum+v2[i]}
a_mean=sum/a.length;
x="v2.mean==a_mean"
if(!assert(x)){alert("Line 65: Failed: "+x)}

x="isNaN(v1.mean)"
if(!assert(x)){alert("Line 67: Failed: "+x)}

//test Property: numNaN
x="v1.numNaN==1"
if(!assert(x)){alert("Line 72: Failed: "+x)}

x="v2.numNaN==0"
if(!assert(x)){alert("Line 75: Failed: "+x)}

//test resize(number size)
if(!assertNoReturn("v1.resize(10)")){alert("Line 84: Failed")};
if(!assert("v1.length==10")){alert("Line 85: Failed")};

//test Property: array
a1=new Array();
for(var i=0;i<v1.length;i++){v1[i]=Math.random(); a1.push(v1[i]);}
if(!assert("sameArray(a1,v1.array)")){alert("Line 89: Failed")}

//test method zero()
zero=new Array();
for(var i=0; i<v1.length;i++){zero.push(0)}
x="v1.zero()"
if(!assertNoReturn(x)){alert("Line 96: Failed: "+x)}
if(!assert("sameArray(zero,v1.array)")){alert("Line 96 Failed: "+x)}

//test method update()
x="v1.update()"
if(!assertNoReturn(x)){alert("Line 101: Failed: "+x)}

//test method interpolate(i,n)
seq=new Array();
seq.push(1);seq.push(2);seq.push(3);
data=new Vector(seq);
x="data.interpolate(0,5)"
if(!assert(x)==1){alert("Line 108: Failed: "+x)}

x="data.interpolate(1,5)"
if(!assert(x)==1.5){alert("Lint 111: Failed: "+x)}

x="data.interpolate(2,5)"
if(!assert(x)==2){alert("Line 114: Failed: "+x)}

x="data.interpolate(3,5)"
if(!assert(x)==2.5){alert("Line 117: Failed: "+x)}

x="data.interpolate(4,5)"
if(!assert(x)==3){alert("Line 120: Failed: "+x)}

//return test result
if(err>0){
alert(" "+err+"failed TestCases");
}
else
alert("All Test Past");

//help functions
function sameArray(a1,a2){
same=true;
for(var i=0; i<v1.length;i++){
if(a1[i]!=a2[i])
{
same=false;
break;
}
}
return same;
}



