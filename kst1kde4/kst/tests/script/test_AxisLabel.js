//unit test for the AxisLabel Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

p=new Plot(new Window())
y=p.yAxis;

//text
assert("y.title.text=\"\int Sin([C_PI]*t)dt\"",1);
if(confirm("Does the intergration sign appear in yAxis label?")!=3)
{
err++;
failed.push("LaTeX command does not work for label text.\n")
}
if(confirm("Does the pi value appear in yAxis label?")!=3)
{
err++;
failed.push("AxisLabel cannot display Kst scalars.\n")
}

//font
assert("y.title.font",2);
assert("y.title.font=\"Luxi Mono\"",3);
assertTrue("y.title.font==\"Luxi Mono\"",4);

//fontSize
assert("y.title.fontSize",5);
assert("y.title.fontSize=12",6);
assertTrue("y.title.fontSize==12",7);

//type
assert("y.title.type==\"Y\"",8);


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
