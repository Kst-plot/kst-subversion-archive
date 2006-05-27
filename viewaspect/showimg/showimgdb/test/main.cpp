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
#include <qdir.h>

QCString prgname;
KInstance *instance = 0;

#define MYDEBUG kdDebug(0)<<__FILE__<<" "<<__LINE__<<" "<<__FUNCTION__<< " "

#ifdef QT_NO_CAST_FROM_ASCII
#warning QT_NO_CAST_FROM_ASCII
#else
#warning NO QT_NO_CAST_FROM_ASCII
#endif

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
	kdDebug(0)<<"THE BEGINNING!"<<endl;
	CategoriesDB *Cdb = new CategoriesDB("sqlite", QDir::homeDirPath()+"/.showimg/MyCategoriesDB3.sidb", QString::null, QString::null, QString::null);

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
			if(op==QString::fromLatin1("catview"))
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
			if(op==QString::fromLatin1("imgview"))
			{
				QPtrList<ImageEntry> imageEntryList=Cdb->imagesSubCategoriesList(args->arg(0));
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op==QString::fromLatin1("imgdel"))
			{
				Cdb->deleteImage(atoi(args->arg(0)));
			}
			else
			if(op==QString::fromLatin1("imgviewpat"))
			{
				QPtrList<ImageEntry> imageEntryList=Cdb->imagesPatternList(args->arg(0));
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op==QString::fromLatin1("imgviewcom"))
			{
				QPtrList<ImageEntry> imageEntryList=Cdb->imagesCommentList(args->arg(0));
				Cdb->printImageEntry(imageEntryList);
			}
			///////////////////////////
			else
			if(op == QString::fromLatin1("catview"))
			{
				Cdb->printSubCategories(args->arg(0));
			}
			else
				if(op == QString::fromLatin1("catadd"))
			{
				Cdb->addTopCategory(args->arg(0));
			}
			else
				if(op == QString::fromLatin1("catdel"))
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
			if(op==QString::fromLatin1("imgadd"))
			{
				Cdb->addImage(args->arg(0), atoi(args->arg(1)));
			}
			else
			if(op==QString::fromLatin1("imgren"))
			{
				Cdb->renameImage(atoi(args->arg(0)), args->arg(1));
			}
			else
			if(op==QString::fromLatin1("imgdelcat"))
			{
				Cdb->deleteCategoryImage(atoi(args->arg(1)), atoi(args->arg(0)));
			}
			else
			if(op==QString::fromLatin1("imgsetnote"))
			{
				Cdb->setImageNote(atoi(args->arg(0)), atoi(args->arg(1)));
			}
			else
			if(op==QString::fromLatin1("imgsetcom"))
			{
				Cdb->setImageComment(atoi(args->arg(0)), args->arg(1));
			}
			else
			if(op==QString::fromLatin1("imgviewnote"))
			{
				QPtrList<ImageEntry> imageEntryList=Cdb->imagesNoteList(atoi(args->arg(0)), atoi(args->arg(1)));
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op==QString::fromLatin1("imgviewdate"))
			{
				QPtrList<ImageEntry> imageEntryList =
						Cdb->imagesDateList(QDate::fromString(args->arg(0),Qt::ISODate), atoi(args->arg(1)));
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op==QString::fromLatin1("imgviewintdate"))
			{
				QPtrList<ImageEntry> imageEntryList =
						Cdb->imagesDateList(QDate::fromString(args->arg(0), Qt::ISODate), QDate::fromString(args->arg(1), Qt::ISODate));
				Cdb->printImageEntry(imageEntryList);
			}
			else
				if(op==QString::fromLatin1("imgaddcat"))
			{
				Cdb->addLink(atoi(args->arg(0)), atoi(args->arg(1)));
			}
			else
			////////////////
			if(op == QString::fromLatin1("catadd"))
			{
				QString desc, msg;
				Cdb->addSubCategory(atoi(args->arg(0)), args->arg(1), desc, msg);
			}
			else
				if(op == QString::fromLatin1("catren"))
			{
				QString msg;
				Cdb->renameCategory(atoi(args->arg(0)), args->arg(1), msg);
			}
			else
				if(op == QString::fromLatin1("catmove"))
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
			if(op==QString::fromLatin1("imgsetdates"))
			{
				Cdb->setImageDate(atoi(args->arg(0)), QDate::fromString(args->arg(1), Qt::ISODate), QDate::fromString(args->arg(2), Qt::ISODate));
				QPtrList<ImageEntry> imageEntryList = Cdb->allImages();
				Cdb->printImageEntry(imageEntryList);
			}
			else
			if(op==QString::fromLatin1("imgaddcat"))
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
	//kdDebug(0)<<"I will delete Cdb->.."<<endl;
	delete(Cdb);
	kdDebug(0)<<"THE END!"<<endl;
}
