//unit test for the PowerSpectrum Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

k=0
N=128;
var n=3/(N-1);
t=new Vector();
t.resize(N);
for(var i=0;i<N;i++){t[i]=k;k=k+n;}//construct a time vector: t is from 0 to 3 with increment 3/128
t.tagName="t";
var e=new Equation("2*Exp(-3*[t])",t)//use t to constuct a function: f(t)=2*e^(-3*t); the power spectrum of this function will be tested
freq=10;

//construct a new power spectrum object
assertNoReturn("ps=new PowerSpectrum(e.yVector,freq)",17)
assertNoReturn("ps.tagName=\"PS-I\"",17);

//test default values of various spectrum properties
assertTrue("ps.length==16",21);
assertTrue("ps.removeMean",22);
assertTrue("ps.average",23);
assertTrue("ps.apodize",24);
assertTrue("ps.frequency==10",25);
assertTrue("ps.vUnits==\"V\"",26);
assertTrue("ps.rUnits==\"Hz\"",27);

//Test modifing values for various properteis
//modify property: frequency
Ts=t[2]-t[1];
Ws=2*Math.PI/Ts;
assertNoReturn("ps.frequency=Ws",33);//modify frequency=50
assertTrue("ps.frequency==Ws",34);
assertNoReturn("ps_c=new PowerSpectrum(e.yVector,Ws)",35)//construct a new Power Spectrum with freqence=Ws
Kst.waitForUpdate();
assertTrue("equalVecs(ps_c.yVector,ps.yVector)",37)//compare the resulting yVectors of the original power spectrum PS-I and the newly created. Their yVectors should be the same 

//modify property: length
assertNoReturn("ps.length=7",40)
assertTrue("ps.length==7",41)
assertNoReturn("ps_c=new PowerSpectrum(e.yVector,Ws,true,7)",42)
Kst.waitForUpdate();
assertTrue("equalVecs(ps_c.yVector, ps.yVector)",44)

//modify property: average
assertNoReturn("ps.average=false",47);
assertFalse("ps.average",45);
assertNoReturn("ps_c=new PowerSpectrum(e.yVector,Ws,false,7)",49)
Kst.waitForUpdate();
assertTrue("equalVecs(ps_c.yVector,ps.yVector)",51)

//modify property:apodize
assertNoReturn("ps.apodize=false",51);
assertFalse("ps.apodize",52);
assertNoReturn("ps_c=new PowerSpectrum(e.yVector,Ws,false,7,false)",56)
Kst.waitForUpdate();
assertTrue("equalVecs(ps_c.yVector,ps.yVector)",58);

//modify property:removeMean
assertNoReturn("ps.removeMean=false",61);
assertFalse("ps.removeMean",62);
assertNoReturn("ps_c=new PowerSpectrum(e.yVector,Ws,false,7,false,false)",63)
ps_c.tagName="PS-C";
Kst.waitForUpdate();
assertTrue("equalVecs(ps_c.yVector,ps.yVector)",66);

//modify property:vUnits
assertNoReturn("ps.vUnits=\"V(PS-I)\"",69)
assertTrue("ps.vUnits==\"V(PS-I)\"",70)

//modify property:rUnits
assertNoReturn("ps.rUnits=\"Hz(ps)\"",73);
assertTrue("ps.rUnits==\"Hz(ps)\"",74);

//compare the visual results
alert("Compare plots of PS-I and PS-C");
w=new Window();
plotcurve(ps.xVector,ps.yVector,ps.rUnits,ps.vUnits,"PS-I",w);
plotcurve(ps_c.xVector,ps_c.yVector,ps_c.rUnits,ps_c.vUnits,"PS-C",w);
if(confirm("Test: Are PS-I and PS-C identical?")!=3)
{
err++;
failed.push("Kst Plots failed\n")
}
//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")
//help functions
//output the plot of the given x and y vector
function plotcurve(xVec,yVec,xUnit,yUnit,topLabel,w)
{
c=new Curve(xVec,yVec)
p=new Plot(w);
p.curves.append(c);
p.xAxis.label=xUnit;
p.yAxis.label=yUnit;
p.topLabel=topLabel;
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
