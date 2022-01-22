#include "dialogs/P2MapPublisher.h"

using namespace ui;

CP2MapPublisher::CP2MapPublisher( QWidget *pParent ) :
	CP2MapPublisher( pParent, false )
{
}

CP2MapPublisher::CP2MapPublisher( QWidget *pParent, bool edit ) :
	QDialog( pParent )
{
	this->setWindowTitle( tr( "Publish File" ) );
	m_edit = edit;
	m_bspHasPTIInstance = false;
	auto pDialogLayout = new QGridLayout( this );
	auto pFindLabel = new QLabel( tr( "Preview Image:" ), this );

	QPixmap tempMap = QPixmap( ":/zoo_textures/SampleImage.png" );
	tempMap = tempMap.scaled( 478.5 / 2, 269.5 / 2, Qt::IgnoreAspectRatio );
	m_pPreviewImage = &tempMap;
	pImageLabel = new QLabel( this );
	pImageLabel->setPixmap( *m_pPreviewImage );
	m_advancedOptionsWindow = new QDialog( this );
	AO = new CP2PublisherAdvancedOptions();
	AO->setupUi( m_advancedOptionsWindow );

	auto pBrowseButton = new QPushButton( this );
	pBrowseButton->setText( tr( "Browse..." ) );
	pBrowseButton->setFixedSize( 478.5 / 2, 20 );

	auto pAdvancedOptionsButton = new QPushButton( this );
	pAdvancedOptionsButton->setText( tr( "Advanced Options." ) );
	pAdvancedOptionsButton->setFixedSize( 478.5 / 2, 40 );

	pSteamToSAgreement = new QCheckBox( tr( "I accept the terms of the Steam Workshop Contribution Agreement." ), this );

	auto pAgreementButton = new QPushButton( this );
	pAgreementButton->setText( tr( "View Agreement" ) );
	pAgreementButton->setFixedSize( 478.5 / 2, 20 );

	auto pButtonBox = new QDialogButtonBox( Qt::Orientation::Horizontal, this );
	auto pOKButton = pButtonBox->addButton( tr( m_edit ? "Update" : "Upload" ), QDialogButtonBox::ApplyRole );
	auto pCloseButton = pButtonBox->addButton( tr( "Cancel" ), QDialogButtonBox::RejectRole );

	auto pTitleDescLayout = new QVBoxLayout( this );
	pTitleDescLayout->setSpacing( 10 );
	auto pTitle = new QLabel( tr( "Title:" ), this );
	pTitleLine = new QLineEdit( this );
	auto pDesc = new QLabel( tr( "Description:" ), this );
	pDescLine = new QTextEdit( this );
	pDescLine->setMinimumHeight( 100 );
	pDescLine->setMaximumHeight( 100 );
	auto pFile = new QLabel( tr( "File:" ), this );

	auto pFileLayout = new QHBoxLayout( this );
	pFileEntry = new QLineEdit( this );
	pFileEntry->setReadOnly( true );
	auto pBrowseButton2 = new QPushButton( tr( "Browse..." ), this );
	pFileLayout->addWidget( pFileEntry );
	pFileLayout->addWidget( pBrowseButton2 );

	pTitleDescLayout->addWidget( pTitle, 0, Qt::AlignTop );
	pTitleDescLayout->addWidget( pTitleLine, 0, Qt::AlignTop );
	pTitleDescLayout->addWidget( pDesc, 0, Qt::AlignTop );
	pTitleDescLayout->addWidget( pDescLine, 0, Qt::AlignTop );
	pTitleDescLayout->addWidget( pFile, 0, Qt::AlignTop );
	pTitleDescLayout->addLayout( pFileLayout, 0 );

	pDialogLayout->setHorizontalSpacing( 25 );

	pDialogLayout->addWidget( pFindLabel, 0, 0 );
	pDialogLayout->addWidget( pImageLabel, 1, 0 );
	pDialogLayout->addWidget( pBrowseButton, 2, 0 );
	pDialogLayout->addWidget( pAdvancedOptionsButton, 3, 0 );
	pDialogLayout->addWidget( pSteamToSAgreement, 9, 0, 1, 2, Qt::AlignBottom );
	pDialogLayout->addWidget( pAgreementButton, 10, 0, Qt::AlignBottom );
	pDialogLayout->addWidget( pButtonBox, 12, 0, 1, 2, Qt::AlignLeft );
	pDialogLayout->addLayout( pTitleDescLayout, 0, 1, 8, 8, Qt::AlignTop );

	connect( pBrowseButton, &QPushButton::pressed, this, &CP2MapPublisher::OpenImageFileExplorer );
	connect( pBrowseButton2, &QPushButton::pressed, this, &CP2MapPublisher::OpenBSPFileExplorer );
	connect( pOKButton, &QPushButton::pressed, this, &CP2MapPublisher::onOKPressed );
	connect( pCloseButton, &QPushButton::pressed, this, &CP2MapPublisher::onClosePressed );
	connect( pAdvancedOptionsButton, &QPushButton::pressed, this, &CP2MapPublisher::onAdvancedClicked );
	connect( pAgreementButton, &QPushButton::pressed, this, &CP2MapPublisher::onAgreementButtonPressed );

	this->setLayout( pDialogLayout );
	this->setFixedSize( this->sizeHint() );
	this->setWindowFlag( Qt::WindowContextHelpButtonHint, false );
}

void CP2MapPublisher::onAdvancedClicked()
{
	AO->label_5->setEnabled( m_edit );
	AO->textEdit->setEnabled( m_edit );
	m_advancedOptionsWindow->exec();
}

void CP2MapPublisher::LoadCreatingItem()
{
	SteamAPICall_t hApiCreateItemHandle = SteamUGC()->CreateItem( CP2MapMainMenu::ConsumerID, k_EWorkshopFileTypeCommunity );
	m_CallResultCreateItem.Set( hApiCreateItemHandle, this, &CP2MapPublisher::OnCreateItem );
	StartLoopCall();
}

void CP2MapPublisher::UpdateItem( PublishedFileId_t itemID )
{
	if ( pTitleLine->text().isEmpty() )
	{
		QMessageBox::warning( this, "File Required!", "You don't have a Title, please insert a title.", QMessageBox::Ok );
		return;
	}

	if ( !defaultFileLocBSP.startsWith( "mymaps/" ) )
	{

	QFile file(defaultFileLocBSP);
	QFileInfo info(defaultFileLocBSP);
	if(!file.exists()) {
	    qInfo() << "File does not exist!";
	    return;
	};
	file.open(QFile::ReadOnly);
	if(file.size() > 209715200){
	    qInfo() << "File too large!";
	    return;
	}
	UGCFileWriteStreamHandle_t filewritestreamhandle = SteamRemoteStorage()->FileWriteStreamOpen( (QString("mymaps/") + info.fileName()).toStdString().c_str() );
		if(file.size() > 104857600){
			QByteArray data = file.readAll();
			QByteArray first = data.left(104857600); //gets the first 100mb.
			QByteArray rest = data.right(data.size() - 104857600); //gets the rest.
		qInfo() << SteamRemoteStorage()->FileWriteStreamWriteChunk( filewritestreamhandle, first.constData(), 104857600 );
		qInfo() << SteamRemoteStorage()->FileWriteStreamWriteChunk( filewritestreamhandle, rest.constData(), data.size() - 104857600 );
		} else {
			QByteArray data = file.readAll();
		qInfo() << SteamRemoteStorage()->FileWriteStreamWriteChunk( filewritestreamhandle, data.constData(), data.size());
		}
		qInfo() << SteamRemoteStorage()->FileWriteStreamClose( filewritestreamhandle );

		PublishedFileUpdateHandle_t old_API_Handle = SteamRemoteStorage()->CreatePublishedFileUpdateRequest( itemID );

		SteamRemoteStorage()->UpdatePublishedFileFile( old_API_Handle, (QString("mymaps/") + info.fileName()).toStdString().c_str() );
		
		SteamRemoteStorage()->UpdatePublishedFilePreviewFile(old_API_Handle, defaultFileLocIMG.toStdString().c_str());
		
		SteamAPICall_t call = SteamRemoteStorage()->CommitPublishedFileUpdate( old_API_Handle );
		m_CallOldApiResultSubmitItemUpdate.Set( call, this, &CP2MapPublisher::OnOldApiSubmitItemUpdate );

	}
	UGCUpdateHandle_t hUpdateHandle = SteamUGC()->StartItemUpdate( CP2MapMainMenu::ConsumerID, itemID );

	if ( !m_edit || ( m_edit && QString( m_editItemDetails.m_rgchTitle ).compare( pTitleLine->text() ) ) )
		bool bTitleResult = SteamUGC()->SetItemTitle( hUpdateHandle, pTitleLine->text().toStdString().c_str() );

	if ( !pDescLine->toPlainText().isEmpty() )
		if ( !m_edit || ( m_edit && QString( m_editItemDetails.m_rgchDescription ).compare( pDescLine->toPlainText() ) ) )
			bool bDescResult = SteamUGC()->SetItemDescription( hUpdateHandle, "This was a test from an uploader program a friend wrote" );

	ERemoteStoragePublishedFileVisibility visibility; // AO->checkBox->isChecked() ? k_ERemoteStoragePublishedFileVisibilityPrivate : k_ERemoteStoragePublishedFileVisibilityPublic;
	switch ( AO->comboBox->currentIndex() )
	{
		case 0:
			visibility = k_ERemoteStoragePublishedFileVisibilityPublic;
			break;
		case 1:
			visibility = k_ERemoteStoragePublishedFileVisibilityPrivate;
			break;
		case 2:
			visibility = k_ERemoteStoragePublishedFileVisibilityFriendsOnly;
			break;
		case 3:
			visibility = k_ERemoteStoragePublishedFileVisibilityUnlisted;
			break;
		default:
			visibility = k_ERemoteStoragePublishedFileVisibilityPublic;
			break;
	}
	bool bVisibilityResult = SteamUGC()->SetItemVisibility( hUpdateHandle, visibility );

	std::string descr;
	if(m_edit && !AO->textEdit->toPlainText().isEmpty()){
		descr = AO->textEdit->toPlainText().toStdString();
	}

	SteamParamStringArray_t tags{};

	std::vector<char*> charray;
	QTreeWidgetItemIterator iterator2(AO->treeWidget);
	while (*iterator2) {
		qInfo() << (*iterator2)->text(0);
        charray.push_back( strdup((*iterator2)->text(0).toStdString().c_str()) );
        ++iterator2;
    }
	tags.m_nNumStrings = charray.size();
	tags.m_ppStrings = (const char**) charray.data();

	SteamUGC()->SetItemTags(hUpdateHandle,&tags);
	for(auto& str : charray)
    free(str);

	for(int ind = 0; ind < iCount; ind++){
		SteamUGC()->RemoveItemPreview( hUpdateHandle, ind );
	}

	//SteamUGC()->SetItemPreview(hUpdateHandle,defaultFileLocIMG.toStdString().c_str());

	QTreeWidgetItemIterator iterator3(AO->ImageTree);
	while (*iterator3) {

		if((*iterator3)->data(0,Qt::UserRole).toString().contains("https://steamuserimages-a.akamaihd.net/ugc/")){			
			QImage image = QImage();
			int imageLoc = (*iterator3)->data(1,Qt::UserRole).toInt();
			qInfo() << AO->AdditionalImageArray[imageLoc];
			image.fromData(AO->AdditionalImageArray[imageLoc]);
			image.save("./resources/AdditionImage"+QString(std::to_string(imageLoc).c_str())+".jpg");
			SteamUGC()->AddItemPreviewFile(hUpdateHandle,("./resources/AdditionImage"+QString(std::to_string(imageLoc).c_str())+".jpg").toStdString().c_str(),k_EItemPreviewType_Image);
			} else {
				SteamUGC()->AddItemPreviewFile(hUpdateHandle,((*iterator3)->data(0,Qt::UserRole).toString()).toStdString().c_str(),k_EItemPreviewType_Image);
			}
         ++iterator3;
    }
	
	// SteamUGC()->AddItemPreviewFile


	SteamAPICall_t hApiSubmitItemHandle = SteamUGC()->SubmitItemUpdate( hUpdateHandle, descr.empty() ? nullptr : descr.c_str() );
	m_CallResultSubmitItemUpdate.Set( hApiSubmitItemHandle, this, &CP2MapPublisher::OnSubmitItemUpdate );

}

void CP2MapPublisher::OnCreateItem( CreateItemResult_t *pItem, bool bFailure )
{
	if ( bFailure )
	{
		std::string errMSG = "Your workshop item could not be created, Error code: " + std::to_string( pItem->m_eResult ) + "\nfor information on this error code: https://partner.steamgames.com/doc/api/steam_api#EResult";
		QMessageBox::warning( this, "Item Creation Failure!", errMSG.c_str(), QMessageBox::Ok );
		return;
	}
	UpdateItem( pItem->m_nPublishedFileId );
}

void CP2MapPublisher::OnSubmitItemUpdate( SubmitItemUpdateResult_t *pItem, bool bFailure )
{
	FinishLoopCall();
}

void CP2MapPublisher::OnOldApiSubmitItemUpdate( RemoteStorageUpdatePublishedFileResult_t *pItem, bool pFailure )
{
	qInfo() << pItem->m_eResult << "\n";
}

void CP2MapPublisher::LoadExistingDetails( SteamUGCDetails_t details, uint32 index )
{
	m_editItemDetails = details;
	pFileEntry->setText( details.m_pchFileName );
	pTitleLine->setText( details.m_rgchTitle );
	pDescLine->setText( details.m_rgchDescription );

	switch ( details.m_eVisibility )
	{
		case k_ERemoteStoragePublishedFileVisibilityPublic:
			AO->comboBox->setCurrentIndex(0);
			break;
		case k_ERemoteStoragePublishedFileVisibilityPrivate:
			AO->comboBox->setCurrentIndex(1);
			break;
		case k_ERemoteStoragePublishedFileVisibilityFriendsOnly:
			AO->comboBox->setCurrentIndex(2);
			break;
		case k_ERemoteStoragePublishedFileVisibilityUnlisted:
			AO->comboBox->setCurrentIndex(3);
			break;
		default:
			AO->comboBox->setCurrentIndex(0);
			break;
	}

	m_EditItemIndex = index;

	std::vector<std::string> vector = splitString(details.m_rgchTags,',');
	for(std::string str : vector){
		if(str == "Singleplayer") continue;
	 	auto item = new QTreeWidgetItem( 0 );
	 	item->setText( 0, QString( str.c_str() ) );
	 	AO->treeWidget->addTopLevelItem( item );
	}

	std::string dir = QDir::currentPath().toStdString() + "/resources/" + std::to_string( details.m_nPublishedFileId ) + "_Image0.png";
	SteamAPICall_t res = SteamRemoteStorage()->UGCDownloadToLocation( details.m_hPreviewFile, dir.c_str(), 0 );
	m_CallOldApiResultSubmitItemDownload.Set( res, this, &CP2MapPublisher::OnOldApiSubmitItemDownload );
	StartLoopCall();
	UGCQueryHandle_t hQueryResult = SteamUGC()->CreateQueryUserUGCRequest( SteamUser()->GetSteamID().GetAccountID(), k_EUserUGCList_Published, k_EUGCMatchingUGCType_Items_ReadyToUse, k_EUserUGCListSortOrder_CreationOrderDesc, SteamUtils()->GetAppID(), CP2MapMainMenu::ConsumerID, 1 );
	SteamUGC()->SetReturnAdditionalPreviews(hQueryResult,true);
	SteamAPICall_t hApiQueryHandle = SteamUGC()->SendQueryUGCRequest( hQueryResult );
	m_CallResultSendQueryUGCRequest.Set( hApiQueryHandle, this, &CP2MapPublisher::OnSendQueryUGCRequest );
	StartLoopCall();
	SteamUGC()->ReleaseQueryUGCRequest( hQueryResult );


}

void CP2MapPublisher::OnSendQueryUGCRequest( SteamUGCQueryCompleted_t *pQuery, bool bFailure ){
    qInfo() << "testing";

    SteamUGCDetails_t pDetails {};
    SteamUGC()->GetQueryUGCResult( pQuery->m_handle, m_EditItemIndex, &pDetails );
    //qInfo() << pDetails.m_flScore;

    iCount = SteamUGC()->GetQueryUGCNumAdditionalPreviews( pQuery->m_handle, m_EditItemIndex );
	int imageIndex = 0;
    for ( uint32 i = 0; i < iCount; i++ )
    {
        const uint iUrlSize = 2000;
        char pchUrl[iUrlSize];
        const uint iFileSize = 2000;
        char pchFileName[iFileSize];
        EItemPreviewType pType;

        qInfo() << SteamUGC()->GetQueryUGCAdditionalPreview( pQuery->m_handle, m_EditItemIndex, i,
                                                  pchUrl, iUrlSize, pchFileName,
                                                  iFileSize, &pType );
        QTreeWidgetItem *pItem = new QTreeWidgetItem( 0 );

		if(pType == k_EItemPreviewType_Image){
				qInfo() << pchFileName;
				qInfo() << pchUrl;
				QNetworkAccessManager manager;
				QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(pchUrl)));
				QEventLoop loop;
				QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
				QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
				loop.exec();
				qInfo() << pchUrl;
				//if(imageIndex != 0 ) { //the first image is skipped, from being added as it's the thumbnail image, but still needs to be added to the array.
				pItem->setText( 0, QString( pchFileName ) );
					// qInfo() << pchUrl;
				pItem->setData(0,Qt::UserRole, pchUrl);
				pItem->setData(1,Qt::UserRole, imageIndex);
				AO->ImageTree->addTopLevelItem( pItem );
				//};

				QImage image;
				qInfo() << "Loading Image...";
				QByteArray ba;
				image.loadFromData(reply->readAll());
        		QBuffer buffer(&ba);
        		buffer.open(QIODevice::ReadWrite);
				image.save(&buffer,nullptr,1);
				qInfo() << reply->readAll();

				// QImageReader reader(reply, &ba);
				AO->AdditionalImageArray.push_back(ba);

				delete reply;
				imageIndex++;
		}
		if(pType == k_EItemPreviewType_YouTubeVideo){
				pItem->setText( 0, QString( pchUrl ) );
                AO->treeWidget_2->addTopLevelItem( pItem );
                break;
		}

    }

    FinishLoopCall();
	
}

void CP2MapPublisher::OnOldApiSubmitItemDownload( RemoteStorageDownloadUGCResult_t *pItem, bool pFailure )
{
	std::string filepath = QDir::currentPath().toStdString() + "/resources/" + std::to_string( m_editItemDetails.m_nPublishedFileId ) + "_Image0.png";
	QPixmap tempMap = QPixmap( filepath.c_str() );
	if ( tempMap.isNull() )
		tempMap = QPixmap( ":/zoo_textures/InvalidImage.png" );
	tempMap = tempMap.scaled( 239.25, 134.75, Qt::IgnoreAspectRatio );
	m_pPreviewImage = &tempMap;
	pImageLabel->setPixmap( *m_pPreviewImage );
	FinishLoopCall();
}

void CP2MapPublisher::onAgreementButtonPressed()
{
	QDesktopServices::openUrl( QUrl( "https://store.steampowered.com/subscriber_agreement/" ) );
}

void CP2MapPublisher::OpenImageFileExplorer()
{
	auto opts = QFileDialog::Option::DontUseNativeDialog;
	QString filePath = QFileDialog::getOpenFileName( this, "Open", defaultFileLocIMG, "*.png *.jpg", nullptr, opts );
	if ( filePath.isEmpty() )
		return;
	defaultFileLocIMG = filePath;
	QPixmap tempMap = QPixmap( filePath );
	if ( tempMap.isNull() )
	{
		defaultFileLocIMG = "InvalidImage.png";
		tempMap = QPixmap( ":/zoo_textures/InvalidImage.png" );
	};
	tempMap = tempMap.scaled( 239.25, 134.75, Qt::IgnoreAspectRatio );
	m_pPreviewImage = &( tempMap );
	pImageLabel->setPixmap( *m_pPreviewImage );
}

void CP2MapPublisher::OpenBSPFileExplorer()
{
	auto opts = QFileDialog::Option::DontUseNativeDialog;
	QString filePath = QFileDialog::getOpenFileName( this, "Open", defaultFileLocBSP, "*.bsp", nullptr, opts );
	defaultFileLocBSP = filePath;
	if ( filePath.isEmpty() )
		return;
	if ( !filePath.endsWith( ".bsp" ) )
	{ // sanity check, this shouldn't be possible.
		QMessageBox::warning( this, "Invalid File Selected!", "You don't have a BSP selected! Please select a valid BSP", QMessageBox::Ok );
		return;
	}
	QFile file( filePath );
	if ( !file.open( QIODevice::ReadOnly ) )
	{
		QMessageBox::warning( this, "File Not Available!", "This BSP is not available, could not be read..." );
		return;
	}
	if ( file.size() > 209715200 )
	{
		QMessageBox::warning( this, "File Too Large!", "This BSP is too large, max 200MB." );
		return;
	}
	QDataStream stream { &file };
    QByteArray bArray( file.size(), 0 );
    qint32 bytes = stream.readRawData( bArray.data(), bArray.size() );
    BSPHeaderStruct_t *castedLump = reinterpret_cast<BSPHeaderStruct_t *>( bArray.data() );
    qInfo() << castedLump->m_version;
    if ( castedLump->m_version != 21 )
    {
        QMessageBox::warning( this, "Invalid BSP", "Invalid BSP.\nEither the file is corrupt or the map is not for Portal 2.\n(only works for BSP version 21)" );
        return;
    }
    QString Entities = bArray.data() + castedLump->lumps[0].fileOffset;
    if ( !Entities.contains( "@relay_pti_level_end" ) && !AO->checkBox_3->isChecked() )
    {
        m_bspHasPTIInstance = false;
        QDialog *dialog = new QDialog( this );
        PTIDialogSetup *PTI = new PTIDialogSetup();
        PTI->setupUi( dialog );
        dialog->exec();
        return;
    }
	m_bspHasPTIInstance = true;
	pFileEntry->setText( filePath );
}

void CP2MapPublisher::onOKPressed()
{
	if ( !defaultFileLocBSP.endsWith( ".bsp" ) )
	{
		QMessageBox::warning( this, "No Map Found!", "You don't have a BSP selected! Please select a valid BSP", QMessageBox::Ok );
		return;
	}
	if ( !pSteamToSAgreement->isChecked() )
	{
		QMessageBox::warning( this, "Steam Workshop Contribution Agreement", "You need to accept the Steam Workshop Contribution Agreement to upload your map!", QMessageBox::Ok );
		return;
	}
	QMessageBox::StandardButton reply = QMessageBox::question( this, "Upload Map?", "Are you sure you want to upload " + defaultFileLocBSP + "?", QMessageBox::Yes | QMessageBox::No );
	if ( reply != QMessageBox::Yes )
	{
		return;
	}
	QFile file( defaultFileLocBSP );
	if ( !file.open( QIODevice::ReadOnly ) )
	{
		QMessageBox::warning( this, "File Not Available!", "This BSP is not available, could not be read..." );
		return;
	}
	if ( file.size() > 209715200 )
	{
		QMessageBox::warning( this, "File Too Large!", "This BSP is too large, max 200MB." );
		return;
	}
	QDataStream stream { &file };
    QByteArray bArray( file.size(), 0 );
    qint32 bytes = stream.readRawData( bArray.data(), bArray.size() );
    BSPHeaderStruct_t *castedLump = reinterpret_cast<BSPHeaderStruct_t *>( bArray.data() );
    qInfo() << castedLump->m_version;
    if ( castedLump->m_version != 21 )
    {
        QMessageBox::warning( this, "Invalid BSP", "Invalid BSP.\nEither the file is corrupt or the map is not for Portal 2.\n(only works for BSP version 21)" );
        return;
    }
    QString Entities = bArray.data() + castedLump->lumps[0].fileOffset;
    if ( !Entities.contains( "@relay_pti_level_end" ) && !AO->checkBox_3->isChecked() )
    {
        m_bspHasPTIInstance = false;
        QDialog *dialog = new QDialog( this );
        PTIDialogSetup *PTI = new PTIDialogSetup();
        PTI->setupUi( dialog );
        dialog->exec();
        return;
    }
	if ( m_edit )
		LoadEditItem();
	else
		LoadCreatingItem();
}

void CP2MapPublisher::LoadEditItem()
{
	UpdateItem( m_editItemDetails.m_nPublishedFileId );
}

void CP2MapPublisher::onClosePressed()
{
	this->close();
}

std::vector<std::string> CP2MapPublisher::splitString(const std::string& input, char delimiter) {
    std::stringstream ss{input};
    std::vector<std::string> out;
    std::string token;
    while (std::getline(ss, token, delimiter))
        out.push_back(token);
    return out;
}