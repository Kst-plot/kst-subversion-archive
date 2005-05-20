#include "categories.h"
#include "categoriesdb.h"
#include "imageentry.h"

#include <kdebug.h>
#include <kcmdlineargs.h>
#include <kapplication.h>
#include <kinstance.h>
#include <kiconloader.h>
#include <kaboutdata.h>

#include <qfileinfo.h>

QCString prgname;
KInstance *instance = 0;

#define MYDEBUG kdDebug()<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "


static KCmdLineOptions options[] =
{
        { "op <operation>",
                " view: view all cats",
                0},
        { "+[op_action]", "action", 0},
        KCmdLineLastOption
};

int main(int argc, char **argv)
{
        QFileInfo info=QFileInfo(argv[0]);
        prgname = info.baseName().latin1();

        KCmdLineArgs::init(argc, argv,
                new KAboutData( prgname, "showimg_KexiDBTest",
                        "0.0.1", "", KAboutData::License_GPL,
                        "(c) 2004-2005, Richard.\n",
                        "",
						"http://ric.jalix.org",
                        "ric@jalix.org"
                )
        );
        KCmdLineArgs::addCmdLineOptions( options );
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        instance = new KInstance(prgname);

	//
	kdDebug()<<"THE BEGINNING!"<<endl;
	CategoriesDB *Cdb = new CategoriesDB();	
 
	QString op = args->getOption("op");
	MYDEBUG<<op<<" " << args->count()<<endl;
	switch(args->count())
	{
		case 0:
			if(op=="imgview")
			{
				QPtrList<ImageEntry> imageEntryList=Cdb->allImages();
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op=="catview")
			{
				Cdb->printCategories();
			}
			///////////			
			else
			{
				kdWarning() << "Parametres incorrects"<<endl;
			}
			break;
		case 1:
			if(op=="imgview")
			{
				QPtrList<ImageEntry> imageEntryList=Cdb->imagesSubCategoriesList(args->arg(0));
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op=="imgdel")
			{	
				Cdb->deleteImage(atoi(args->arg(0)));
			}
			else
			if(op=="imgviewpat")
			{
				QPtrList<ImageEntry> imageEntryList=Cdb->imagesPatternList(args->arg(0));
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op=="imgviewcom")
			{
				QPtrList<ImageEntry> imageEntryList=Cdb->imagesCommentList(args->arg(0));
				Cdb->printImageEntry(imageEntryList);
			}
			///////////////////////////
			else
			if(op == "catview")
			{
				Cdb->printSubCategories(args->arg(0));
			}			
			else
				if(op == "catadd")
			{	
				Cdb->addTopCategory(args->arg(0));
			}			
			else
				if(op == "catdel")
			{	
				Cdb->deleteNodeCategory(atoi(args->arg(0)));
			}
			///////////			
			else
			{
				kdWarning() << "Parametres incorrects"<<endl;
			}
			break;
		case 2:
			if(op=="imgadd")
			{	
				Cdb->addImage(args->arg(0), atoi(args->arg(1)));
			}
			else			
			if(op=="imgren")
			{	
				Cdb->renameImage(atoi(args->arg(0)), args->arg(1));
			}			
			else			
			if(op=="imgdelcat")
			{	
				Cdb->deleteCategoryImage(atoi(args->arg(1)), atoi(args->arg(0)));
			}			
			else			
			if(op=="imgsetnote")
			{	
				Cdb->setImageNote(atoi(args->arg(0)), atoi(args->arg(1)));
			}			
			else			
			if(op=="imgsetcom")
			{	
				Cdb->setImageComment(atoi(args->arg(0)), args->arg(1));
			}			
			else
			if(op=="imgviewnote")
			{
				QPtrList<ImageEntry> imageEntryList=Cdb->imagesNoteList(atoi(args->arg(0)), atoi(args->arg(1)));
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op=="imgviewdate")
			{
				QPtrList<ImageEntry> imageEntryList =
						Cdb->imagesDateList(QDate::fromString(args->arg(0),Qt::ISODate), atoi(args->arg(1)));
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op=="imgviewintdate")
			{
				QPtrList<ImageEntry> imageEntryList =
						Cdb->imagesDateList(QDate::fromString(args->arg(0), Qt::ISODate), QDate::fromString(args->arg(1), Qt::ISODate));
				Cdb->printImageEntry(imageEntryList);
			}
			else
				if(op=="imgaddcat")
			{	
				Cdb->addLink(atoi(args->arg(0)), atoi(args->arg(1)));
			}
			else
			////////////////
			if(op == "catadd")
			{	
				QString desc, msg;
				Cdb->addSubCategory(atoi(args->arg(0)), args->arg(1), desc, msg);
			}
			else			
				if(op == "catren")
			{	
				QString msg;
				Cdb->renameCategory(atoi(args->arg(0)), args->arg(1), msg);
			}			
			else			
				if(op == "catmove")
			{	
				Cdb->moveCategory(atoi(args->arg(0)), atoi(args->arg(1)));
			}
			///////////////////
			else
			{
				kdWarning() << "Parametres incorrects"<<endl;
			}
			break;
			
		case 3:
			if(op=="imgsetdates")
			{	
				Cdb->setImageDate(atoi(args->arg(0)), QDate::fromString(args->arg(1), Qt::ISODate), QDate::fromString(args->arg(2), Qt::ISODate));
				QPtrList<ImageEntry> imageEntryList = Cdb->allImages();
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op=="imgaddcat")
			{	
				Cdb->addLink(args->arg(0), args->arg(1), args->arg(2));
			}
			else
			{
				kdWarning() << "Parametres incorects"<<endl;
			}
			break;
		default:
			QPtrList<ImageEntry> imageEntryList = Cdb->allImages();
			Cdb->printImageEntry(imageEntryList);
			
			Cdb->printCategories();

	}
	//kdDebug()<<"I will delete Cdb->.."<<endl;
	delete(Cdb);
	kdDebug()<<"THE END!"<<endl;
}
