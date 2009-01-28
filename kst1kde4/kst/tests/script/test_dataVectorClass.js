//unit test for the DataVector Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

B=new Array();
//prompt the user to enter ascii datasource file name for testing 
noerr=true;
while((s=prompt("Enter a ascii datasource file name or \"q\" to exit"))!="q"&&noerr)
{
sr=new DataSource(s);
	if(sr.empty)
	{
	failed.push("Empty data file"+"\n")
	break;
	}

list=sr.fieldList();
N=sr.frameCount(list[1]);


//test constructor
B.push(assert("v1=new DataVector(sr,list[0])"),23);
B.push(assert("v2=new DataVector(sr,list[1],0,-1,2,true)"),24);

//test valid
B.push(assertTrue("v1.valid"),25);
B.push(assertTrue("v2.valid"),26);

//test skip
B.push(assertFalse("v1.skip"),27);
B.push(assertTrue("v2.skip"),28);

//test boxcar
B.push(assertFalse("v1.boxcar"),35);
B.push(assertTrue("v2.boxcar"),36);

//test skipLength
B.push(assertTrue("v1.skipLength==0"),39);
B.push(assertTrue("v2.skipLength==2"),40);

//test startFrame
B.push(assertTrue("v1.startFrame==0"),43);
B.push(assertTrue("v2.startFrame==0"),44);

//test frames
B.push(assert("v1.frames"),47);
B.push(assert("v2.frames"),48);

//test changeFrames(start,count,[skip,[ave]])
B.push(assert("v2.changeFrames(-1,N,2,true)"),51);

//readToEnd
//count>0
B.push(assert("v=new DataVector(sr,list[1],1,5)"),55);
B.push(assertFalse("v.readToEnd"));

//count<0
B.push(assert("v=new DataVector(sr,list[1],1,-1)"),59);
B.push(assertTrue("v.readToEnd"),60);


//countFromEnd
//start>0
B.push(assert("v=new DataVector(sr,list[1],1,5)"),65);
B.push(assertFalse("v.countFromEnd"),66);

//start<0
B.push(assert("v=new DataVector(sr,list[1],-1,5)"),69);
B.push(assertTrue("v.countFromEnd"),70);

//startFrameRequested
B.push(assert("v=new DataVector(sr,list[1],1,5)"),73);
B.push(assertTrue("v.startFrameRequested==1"),74);

B.push(assert("v=new DataVector(sr,list[1],-1,5)"),76);
B.push(assertTrue("v.startFrameRequested==-1"),77);

//framesRequested
B.push(assert("v=new DataVector(sr,list[1],-1,5)"),80);
B.push(assertTrue("v.framesRequested==5"),81);
B.push(assert("v=new DataVector(sr,list[1],1,-1)"),82);
B.push(assertTrue("v.framesRequested==-1"),83);

  if(!alltrue(B))
  {
   failed.push("Failed test file: "+s+"\n");
   noerr=false;
   break;
  }
}

//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")

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
