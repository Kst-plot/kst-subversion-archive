//unit test of Equation Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

//construct a sequence: -10,-9.5,-9,...,9,9.5,10
var seq=new Array();
k=-10;
for(var i=0;i<=40;i++)
{
  seq.push(k);
  k+=0.5;
}

//construct a random array
var arr=new Array();
for(var i=0;i<=40;i++)
{
   arr.push(Math.random());
}

//test constructor Equation(equation,xVector[,interpolate])
var xa=new Vector(arr);
xa.tagName="xa";
Ea="[xa]>0.5";
assertNoReturn("eq_a=new Equation(Ea,xa)",26);
wait();
//test constructor Equation(equation,x0,x1,n)
var xb=new Vector(seq);
xb.tagName="xb";
Eb="2*Sin([xb])+3*Cos([xb])";
assertNoReturn("eq_b=new Equation(Eb,-10,10,41)",32)
wait();

//test property valid
assertTrue("eq_a.valid",36)
assertTrue("eq_b.valid",37)
//test property xVector
assertTrue("equalVecs(xa,eq_a.xVector)",39);
assertTrue("equalVecs(xb,eq_b.xVector)",40);

//test Property yVector
var yb=new Vector(seq);
for(var i=0;i<yb.length;i++)
{
	yb[i]=2*Math.sin(xb[i])+3*Math.cos(xb[i]);
}
assertTrue("equalVecs(yb,eq_b.yVector)",48)

//test Property equation
assertTrue("eq_a.equation==Ea",51)
assertTrue("eq_b.equation==Eb",52)
nEb="2*Sin([xb])-3*Cos([xb])"
assertNoReturn("eq_b.equation=nEb",54)//change the eqaution expression to a new one
wait();
assertTrue("eq_b.equation==nEb",56)
var nyb=new Vector(seq);
for(var i=0;i<nyb.length;i++)
{
	nyb[i]=2*Math.sin(xb[i])-3*Math.cos(xb[i]);
}
assertTrue("equalVecs(eq_b.yVector,nyb)",62)


//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Test Past\n")

//help functions
function wait()
{
var time=0
while(time!=500000)
time++;
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
function assertNoReturn(x,line){
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
            failed.push(x+"\n")
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
                   failed.push("Line "+ line+" "+x+"\n") 
                 }
           }
	catch(e){
		alert("Line: "+line+"Error: " + e.name + "\nLast test was: " + x)
		err++;
                failed.push(x+"\n")
		correct=false;
        } 
        finally{
		return correct;
         }
}
