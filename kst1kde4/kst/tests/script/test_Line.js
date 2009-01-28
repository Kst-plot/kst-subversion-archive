//unit test for the Line Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

p=new Plot(new Window());

//test constructors
assert("l=new Line(\"W1\")");
assert("l=new Line(p)");

//from and to
assert("l.from=new Point(100,100)");
Kst.waitForUpdate();
assertTrue("l.from.x==100");
assertTrue("l.from.y==100");

assert("l.to=new Point(300,300)");
Kst.waitForUpdate();
assertTrue("l.to.x==300");
assertTrue("l.to.y==300");

//width
assertTrue("l.width==1");
assert("l.width=3");
Kst.waitForUpdate();
assertTrue("l.width==3");

//capStyle
assert("l.capStyle=0");
assertTrue("l.capStyle==0");
assert("l.capStyle=1");
assertTrue("l.capStyle==1");
assert("l.capStyle=2");
assertTrue("l.capStyle==2");

//lineStyle
assert("l.lineStyle=0");
assertTrue("l.lineStyle==0");
assert("l.lineStyle=1");
assertTrue("l.lineStyle==1");
assert("l.lineStyle=2");
assertTrue("l.lineStyle==2");
assert("l.lineStyle=3");
assertTrue("l.lineStyle==3");
assert("l.lineStyle=4");
assertTrue("l.lineStyle==4");

//return the test result
failed.push(err+" failed test cases found");
if(err>0)
alert(failed)
else
alert("All Tests Past\n")


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
