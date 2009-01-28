//unit test for the Legend Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

v0=staticV(0,10,11);
v0.tagName="x";
v1=staticV(0,10,11);
v1.tagName="y";
v2=staticV("1,2,11");
v2.tagName="y2";
c1=new Curve(v0,v1);
c1.tagName="c1";
c2=new Curve(v0,v2);
c2.tagName="c2";

//test constructors
assert("l1=new Legend(new Window())");
p=new Plot(new Window());
assert("l2=new Legend(p)");

//test addCurve(curve)
assert("l1.addCurve(c1)");
assert("l1.addCurve(c2)");
assert("l2.addCurve(\"c1\")");
assert("l2.addCurve(\"c2\")");

//test font
assert("l2.font=\"Luxi Mono\"");
assertTrue("l2.font==\"Luxi Mono\"");

//test fontSize
assert("l2.fontSize=12");
assertTrue("l2.fontSize==12");

//test textColot
assert("l2.textColor=\"blue\"");
assertTrue("l2.textColor==\"#0000ff\"");
assert("l1.textColor=\"red\"");
assertTrue("l1.textColor==\"#ff0000\"");

//vertical
assertTrue("l2.vertical");
assert("l2.vertical=false");
assertFalse("l2.vertical");

//curves
assertTrue("l2.curves.length==2");

//title
assert("l2.title=\"l2-Legend\"");
assertTrue("l2.title==\"l2-Legend\"");

//test removeCurve(curve)
assert("l2.removeCurve(c1)");
assert("l2.removeCurve(c2)");
assertTrue("l2.curves.length==0");

assert("l1.removeCurve(\"c1\")");
assert("l1.removeCurve(\"c2\")");
assertTrue("l1.curves.length==0");


//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")

//get a static vector
function staticV(left,right,N){
d=(right-left)/(N-1);
a=new Array();
e=left;

for(var i=0; i<N;i++){
a.push(e);
e=e+d;
}
v=new Vector(a);
return v;
}

//get an equation vector
function equationV(x,equation){
eq=new Equation(equation,x);
return eq.yVector;
}
//compare if two vectors have the same entries
function equalVecs(v1,v2)
{
isequal=true;
if(v1.length!=v2.length)
isequal=false;
 else
 {
   for(var i=0;i<v1.length;i++)
   {
       v1=Math.floor(v1[i]*1E10)
       v2=Math.floor(v2[i]*1E10)
      if(v1!=v2)
       {
         isequal=false;
         break;
        }	
   }	
 }
return isequal;
}
//assert code: x works properly without any error
function assert(x,line){
correct=true;
	try{
		eval(x);
         } catch (e) {
		err++;
                failed.push("Number: "+line+" "+x+"\n")
		alert("Number: "+line+" Error: " + e.name + "\nLast test was: " + x);
		
		correct=false;
         }
         finally
            {
              return correct;
            }

}

//assert code: x evalutes to true
function assertTrue(x,line){
correct=true;
	try{
		
		if(!eval(x))
                 { 
                   correct=false;
                   err++;
                   failed.push("Number: "+ line+" "+x+"\n")
                 }   
	}
        catch(e){
            alert("Number: "+line+"Error: " + e.name + "\nLast test was: " + x);
            err++;
            failed.push("Number: "+line+" "+x+"\n")
            correct=false; 
        } 
        finally{
                return correct;
             }
}

/*assert code x evalutes to false*/
function assertFalse(x,line){
correct=true;
	try{
		if(eval(x))
                 {
                   correct=false;
                   err++;
                   failed.push("Number: "+ line+" "+x+"\n") 
                 }
           }
	catch(e){
		alert("Number: "+line+"Error: " + e.name + "\nLast test was: " + x)
		err++;
                failed.push("Number: "+line+x+"\n")
		correct=false;
        } 
        finally{
		return correct;
         }
}
