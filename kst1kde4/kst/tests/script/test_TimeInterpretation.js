//unit test for the AxisLabel Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

p=new Plot(new Window())
i=p.xAxis.interpretation//get an TimeInterpretation object stored in i

//test active------------------Number
assertFalse("i.active",		1);
assert("i.active=true",		2);
assertTrue("i.active",		3);

//test input-------------------Number
assertTrue("i.input==0",	4);
assert("i.input=1",		5);
assertTrue("i.input==1",	6); 
assert("i.input=2",		7);
assertTrue("i.input==2",	8);
assert("i.input=3",		9);
assertTrue("i.input==3",	10);
assert("i.input=4",		11);
assertTrue("i.input==4",	12);
assert("i.input=5",		13);
assertTrue("i.input==5",	14);

//test output-------------------Number
assertTrue("i.output==0",	15);
assert("i.output=1",		16);
assertTrue("i.output==1",	17); 
assert("i.output=2",		18);
assertTrue("i.output==2",	19);
assert("i.output=3",		20);
assertTrue("i.output==3",	21);
assert("i.output=4",		22);
assertTrue("i.output==4",	23);
assert("i.output=5",		24);
assertTrue("i.output==5",	25);
assert("i.output=6",		26);
assertTrue("i.output==6",	27);
assert("i.output=7",		28);
assertTrue("i.output==7",	29);

//test axisType
assertTrue("i.axisType==\"X\"",	30);
iy=p.yAxis.interpretation
assertTrue("iy.axisType==\"Y\"",31);
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
