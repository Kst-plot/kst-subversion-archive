//test the Plot Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

/*constructors*/
assert("p=new Plot(\"W1\")");

v1=staticV(-10,10,41); v1.tagName="x";
v2=equationV(v1,"Sin([x])");
v3=equationV(v1,"Cos([x])");

/*Test Property curves*/
assert("p.curves");
/*Test Property xAxis,yAxis, title*/
assert("p.xAxis",25);
assert("p.yAxis",26);
assert("p.title",27);
/*Test Propety:tied*/
assertFalse("p.tied",31);
assert("p.tied=true",32);
assertTrue("p.tied",33);
/*Test Property:legend*/
assertTrue("p.lengend==null");

/*Test Property:createLegend*/
assert("p.createLegend()");


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
