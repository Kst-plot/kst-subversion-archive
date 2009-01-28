//unit test for the DataSource Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

//prompt the user to enter ascii datasource file name for test
noerr=true;
while((s=prompt("Enter a ascii datasource file name or \"q\" to exit"))!="q"&&noerr)
{
if(
assert("sr=new DataSource(s)",11)&&
assertTrue("sr.valid" ,12)&&
assertFalse("sr.empty" ,13)&&
assertTrue("sr.completeFieldList" ,14)&&
assertTrue("sr.fileName==s" ,15)&&
assertTrue("sr.fileType==\"ASCII\"",16)&&
assertTrue("sr.source==\"ASCII File Reader\"",17)&&
assertTrue("sr.fieldList().length>0",18)&&
assertTrue("sr.matrixList().length>=0",19))
noerr=true;
else
{
failed.push("Failed case: "+s+"\n")
break;
}
for(var i=0;i<sr.fieldList().length;i++)
{
field=sr.fieldList()[i];
if(
assertTrue("sr.isValidField(field)",30)&&
assert("sr.samplesPerFrame(field)",31)&&
assert("sr.frameCount(field)",32))
noerr=true;
else
{
failed.push("Failed case: "+s+"\n")
break;
}
}
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
