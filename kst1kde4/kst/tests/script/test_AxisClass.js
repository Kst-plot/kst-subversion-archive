//test the Plot Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")


//test property:log
p=new Plot(new Window());
assertFalse("p.xAxis.log");
assert("p.xAxis.log=true");
assertTrue("p.xAxis.log");

//suppressed
p=new Plot(new Window());
assert("p.xAxis.suppressed=false");
assertFalse("p.xAxis.suppressed");
assert("p.xAxis.suppressed=true");
assertTrue("p.xAxis.suppressed");

//oppositeSuppressed
p=new Plot(new Window());
assert("p.xAxis.oppositeSuppressed",19)
assert("p.xAxis.oppositeSuppressed=false",22);
assertFalse("p.xAxis.oppositeSuppressed",23);
assert("p.xAxis.oppositeSuppressed=true",24);
assertTrue("p.xAxis.oppositeSuppressed",25);

//offsetMode
p=new Plot(new Window());
assert("p.xAxis.offsetMode",30);
assert("p.xAxis.offsetMode=false",31);
assertFalse("p.xAxis.offsetMode",32);
assert("p.xAxis.offsetMode=true",33);
assertTrue("p.xAxis.offsetMode",34)

//reversed
p=new Plot(new Window());
assert("p.xAxis.reversed",38);
assert("p.xAxis.reversed=false",39);
assertFalse("p.xAxis.reversed",40);
assert("p.xAxis.reversed=true",41);
assertTrue("p.xAxis.reversed",42);

//majoarGridLines
p=new Plot(new Window())
assertFalse("p.xAxis.majorGridLines",46);
assert("p.xAxis.majorGridLines=true",47);
assertTrue("p.xAxis.majorGridLines",48);


//majorGridColor
assert("p.xAxis.majorGridColor=\"blue\"",52);
assertTrue("p.xAxis.majorGridColor==\"#0000ff\"",53);


//minorGridLines
p=new Plot(new Window())
assertFalse("p.xAxis.minorGridLines",58);
assert("p.xAxis.minorGridLines=true",59);

//minorGridColor
assert("p.xAxis.minorGridColor=\"green\"",62);
assertTrue("p.xAxis.minorGridColor==\"#00ff00\"",63);

//innerTicks
p=new Plot(new Window());
assertTrue("p.xAxis.innerTicks",67);
assert("p.xAxis.innerTicks=false",68);
assertFalse("p.innerTicks",69);

//outerTicks
p=new Plot(new Window())
assert("p.xAxis.outerTicks",73);
assert("p.xAxis.outerTicks=true",74);
assertTrue("p.xAxis.outerTicks",75);

//label
p=new Plot(new Window())
assert("p.xAxis.label",79);
assert("p.xAxis.label=\"x\"",80);
assertTrue("p.xAxis.label==\"x\"",81);

//type
assertTrue("p.xAxis.type==\"X\"",84);

//minorTickCount
p=new Plot(new Window())
assert("p.xAxis.minorTickCount=1",88);
assertTrue("p.xAxis.minorTickCount==1",89);
assert("p.xAxis.minorTickCount=-1",90);

//majorTickDensity
assertTrue("p.xAxis.majorTickDensity==1",93);
assert("p.xAxis.majorTickDensity=0",94);
assertTrue("p.xAxis.majorTickDenstiy==0",95);
assert("p.xAxis.majorTickDensity=2",96);
assertTrue("p.xAxis.majorTickDenstiy==2",97);
assert("p.xAxis.majorTickDensity=3",98);
assertTrue("p.xAxis.majorTickDensity==3",99);

//scaleMode
assert("p.xAxis.scaleMode",102);

//interpretation
assert("p.xAxis.interpretation",105);

//title
assert("p.xAxis.title",108);

//tickLabel
assert("p.xAxis.tickLabel",111);

//test method scaleRange(number min,number max)
assert("p.xAxis.scaleRange(0.2,0.6)",114);
assertTrue("p.xAxis.scaleMode==2",115);

//test method scaleAutoSpikeInsensitive()
assert("p.xAxis.scaleAutoSpikeInsensitive()",118);
assertTrue("p.xAxis.scaleMode==4",119);

//test method scaleExpression()
assert("p.xAxis.scaleExpression(\"2*x\",\"4*x\")",122);
assertTrue("p.xAxis.scaleMode==6",123);

//test method scaleAuto
assert("p.xAxis.scaleAuto()",126);
assertTrue("p.xAxis.scaleMode==0",127);


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
