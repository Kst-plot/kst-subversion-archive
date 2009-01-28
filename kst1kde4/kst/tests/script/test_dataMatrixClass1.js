//unit test for the DataMatrix Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")


B=new Array();
//prompt the user to enter ascii datasource file name for test
noerr=true;
while((s=prompt("Enter an ascii matrix datasource file name or \"q\" to exit"))!="q"&&noerr)
{
sr=new DataSource(s);
if(sr.matrixList().length==0||sr.empty||!sr.valid)
{
alert("Not valid data Source");
break;
}
f=sr.matrixList()[0];
Kst.waitForUpdate();
B.push(assert("mx=new DataMatrix(sr,f,1,1,10,2,2,true)"),14)
Kst.waitForUpdate();
B.push(assertTrue("mx.valid"),15)
B.push(assertTrue("mx.skip"),16)
B.push(assertTrue("mx.boxcar"),17)
B.push(assertFalse("mx.xReadToEnd"),18)
B.push(assertFalse("mx.yReadToEnd"),19)
B.push(assertFalse("mx.xCountFromEnd"),20)
B.push(assertFalse("mx.yCountFromEnd"),21)
B.push(assertTrue("mx.field==f"),22);

B.push(assert("mx=new DataMatrix(sr,f,1,1,-1,-1)"),24)
B.push(assertFalse("mx.skip"),25)
B.push(assertFalse("mx.boxcar"),26)
B.push(assertTrue("mx.xReadToEnd"),27);
B.push(assertTrue("mx.yReadToEnd"),28);

B.push(assert("mx=new DataMatrix(sr,f,-1,-1,5,5)"),33)
B.push(assertTrue("mx.xCountFromEnd"),31)
B.push(assertTrue("mx.yCountFromEnd"),32)

B.push(assert("mx.change(1,1,10,2,2,true)"));
B.push(assert("mx.reload()"));

	if(!alltrue(B))
	{
		failed.push("Falied case: " + s+"\n")
                noerr=false;
		break;
	}
}

//if all booleans are true
function alltrue(B)
{
all=true;
 for(var i=0;i<B.length;i++)
 {
	if(!B[i])
	{
	all=false;
	break;
	}
 }
return all;
}

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
