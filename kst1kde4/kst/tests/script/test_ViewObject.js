//unit test for the ViewObject Class
err=0;
failed=new Array();
failed.push("Failed codes: \n")

plot=new Plot(new Window());
box=new Box(new Window());
ellipse=new Ellipse(new Window());


//resize(size)
size=new Size(200,200)
assert("plot.resize(size)");
assert("box.resize(size)");
assert("ellipse.resize(size)");

//size
W=size.w; H=size.h
assertTrue("plot.size.w==W&&plot.size.h==H");
assertTrue("box.size.w==W&&box.size.h==H");
assertTrue("ellipse.size.w==W&&ellipse.size.h==H");

//move(pos)
point=new Point(300,300);
assert("plot.move(point)");
assert("box.move(point)");
assert("ellipse.move(point)");

//position
X=point.x;
Y=point.y;
assertTrue("plot.position.x==X&&plot.position.y==Y");
assertTrue("box.position.x==X&&box.position.y==Y");
assertTrue("ellipse.position.x==X&&ellipse.position.y==Y");

//findChild(pos)
p=new Plot(new Window());
b=new Box(p);
//the returned child and box b should share the same properties
assert("child=p.findChild(new Point(1,1))");
assertTrue("child.xRound==b.xRound");
assertTrue("child.yRound==b.yRound");
assertTrue("child.conrnerStyle==b.cornerStyle");
assertTrue("child.borderWidth==b.borderWidth");
assertTrue("child.borderColor==b.borderColor");

assertTrue("p.findChild(new Point(100,100))==null");

//convertTo(type)
a=new Arrow(p);
l=new Line(p)
assertFalse("a.convertTo(\"Line\")==null");
assertTrue("l.convertTo(\"Arrow\")==null");

//raiseToTop()
p=new Plot(new Window());
b=new Box(p);
e=new Ellipse(p);
l=new Label(p);
b.resize(new Size(100,100));
e.resize(new Size(100,200));
b.borderWidth=2;
b.borderColor="green";
e.borderWidth=2;
e.borderColor="pink";
l.text="Label";

assert("b.raiseToTop()");
assert("l.raiseToTop()");
assert("e.raiseToTop()");

//lowerToBottom()
assert("e.lowerToBottom()");
assert("l.lowerToBottom()");
assert("b.lowerToBottom()");

//raise() and lower()
assert("e.raise()");
assert("e.lower()");
assert("l.raise()");
assert("l.lower()");
assert("b.raise()");
assert("b.lower()");

//transparent
assertFalse("e.transparent")
assertFalse("b.transparent");
assert("e.transparent=true");
assert("b.transparent=true");
assertTrue("e.transparent");
assertTrue("e.transparent");

//color
assert("l.color=\"red\"");
assertTrue("l.color==\"#ff0000\"");
assert("p.color=\"orange\"");
assertTrue("p.color==\"#ffa500\"");

//maximized
assert("b.maximized=true");
assertTrue("b.maximized");
assert("b.maximized=false");
assertFalse("b.maximized");

//minimumSize
assert("b.minimumSize");
assert("e.minimumSize");
assert("l.minimumSize");
assert("p.minimumSize");

//type
assertTrue("p.type==\"Plot\"");
assertTrue("b.type==\"Box\"");
assertTrue("e.type==\"Ellipse\"");

//children
assertTrue("p.children.length==3");
assertTrue("p.children[0].xRound==b.xRound");
assertTrue("p.children[0].yRound==b.yRound");
assertTrue("p.children[0].cornerStyle==b.cornerStyle");
assertTrue("p.children[0].borderWidth==b.borderWidth");
assertTrue("p.children[0].borderColor==b.borderColor");

assertTrue("p.children[1].text==l.text");
assertTrue("p.children[1].font==l.font");
assertTrue("p.children[1].fontSize==l.fontSize");

assertTrue("p.children[2].borderWidth==e.borderWidth");
assertTrue("p.children[2].borderColor==e.borderColor");

//onGrid.columns
assertFalse("p.onGrid")
assert("p.columns=2");
assertTrue("p.columns==2");
assertTrue("p.onGrid");

//remove()
assert("l.remove()");
assert("b.remove()");
assert("e.remove()");


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
