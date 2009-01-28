//unit test for the Collection Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

while((s=prompt("Enter a Kst file name or \"q\" to exit"))!="q")
{
Kst.document.load(s);
assertTrue("Kst.dataSources.readOnly",9);
assertTrue("Kst.dataSources.length>=0",10);
assertTrue("Kst.scalars.readOnly",11);
assertTrue("Kst.scalars.length>=0",12);
assertTrue("Kst.strings.readOnly",13);
assertTrue("Kst.strings.length>=0",14);
assertTrue("Kst.vectors.readOnly",15);
assertTrue("Kst.vectors.length>=0",16);
assertTrue("Kst.matrices.readOnly",17);
assertTrue("Kst.matrices.length>=0",18);
assertTrue("Kst.windows.readOnly",19);
assertTrue("Kst.windows.length>=0",20);
assertTrue("Kst.objects.readOnly",21);
assertTrue("Kst.objects.length>=0",22);
}

p=new Plot(new Window("CurveCollection_Test"))
v1=staticV(-10,10,1000);
v1.tagName="v1";
assert("cs=p.curves")
for(var i=0;i<5;i++)
{
e="Sin("+i+"*[C_PI]*[v1])"
v=equationV(v1,e);
c=new Curve(v1,v);
assert("cs.append(c)");
}
assertFalse("cs.readOnly")
assertTrue("cs.length==5");
assert("cs.remove(c)");
assertTrue("cs.length==4");
assert("cs.remove(0)");
assertTrue("cs.length==3");
assert("cs.clear()");
assertTrue("cs.length==0");

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
//assert code: x works properly without any error
function assert(x,line){
correct=true;
	try{
		eval(x);
         } catch (e) {
		err++;
                failed.push("Line: "+line+" "+x+"\n")
		alert("Line: "+line+" Error: " + e.name + "\nLast test was: " + x);
		
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
                   failed.push("Line: "+ line+" "+x+"\n")
                 }   
	}
        catch(e){
            alert("Line: "+line+"Error: " + e.name + "\nLast test was: " + x);
            err++;
            failed.push("Line: "+line+" "+x+"\n")
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
                   failed.push("Line: "+ line+" "+x+"\n") 
                 }
           }
	catch(e){
		alert("Line: "+line+"Error: " + e.name + "\nLast test was: " + x)
		err++;
                failed.push("Line: "+line+x+"\n")
		correct=false;
        } 
        finally{
		return correct;
         }
}
