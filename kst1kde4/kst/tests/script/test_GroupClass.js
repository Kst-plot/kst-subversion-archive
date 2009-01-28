//unit test for the Group Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")



viewObs=setupTest();
p=viewObs.pop();
l=viewObs.pop();
e=viewObs.pop();
b=viewObs.pop();

//test constructors
assert("g=new Group(new Window())")
assert("g=new Group(p)");

//test append()
assert("g.append(l)");
assert("g.append(e)");
assert("g.append(b)");

//test ungroup()
assert("g.ungroup()");

//test prepend()
assert("g.prepend(l)");
assert("g.prepend(e)");
assert("g.prepend(b)");

function setupTest(){
p=new Plot("W1");
b=new Box(p);
b.resize(100,100);
b.borderWidth=2;
b.transparent=true;
b.borderColor="pink";
e=new Ellipse(p);
e.resize(100,100);
e.borderWidth=2;
e.borderColor="orange";
e.transparent=true;
l=new Line(p);
l.to=new Point(50,50);
var viewObs=new Array();

viewObs.push(b);
viewObs.push(e);
viewObs.push(l);
viewObs.push(p);
return viewObs;
}


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
