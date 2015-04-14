/****************************************************************************
 * Copyright (C) 2011
 * by Dj_Skual
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * FileMiiWindow.cpp
 *
 * for SaveGame Manager GX 2011
 ***************************************************************************/
#include <unistd.h>

#include "FileMiiWindow.hpp"
#include "../Files/CFileList.hpp"
#include "../Themes/CTheme.h"
#include "../Prompts/SelectBrowser.h"
#include "../Settings/CSettings.h"
#include "../EmuNand/CEmuMiiList.hpp"
#include "../Miis/CMiiList.hpp"
#include "../Language/gettext.h"
#include "../Menu/menus.h"

#define NONE            0
#define LEFT            1
#define RIGHT           2
#define IN              3
#define OUT             4

FileMiiWindow::FileMiiWindow(int Selected)
	: GuiWindow(572, 372)
{
	returnVal = -1;
	miiSelected = Selected;
	refreshNeeded = false;
	action = 0;
	
	SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	SetPosition(6, 0);

	imgDialogBox = Resources::GetImageData("manage_save_bg.png");
	imgButtonBox = Resources::GetImageData("manage_save_button_box.png");
	imgClose = Resources::GetImageData("button_close.png");
	imgCloseOver = Resources::GetImageData("button_close_over.png");
	imgLeft = Resources::GetImageData("arrow_left.png");
	imgRight = Resources::GetImageData("arrow_right.png");

	trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigB = new GuiTrigger();
	trigB->SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);
	trigHome = new GuiTrigger();
	trigHome->SetButtonOnlyTrigger(-1, WiiControls.HomeButton | ClassicControls.HomeButton << 16, GCControls.HomeButton);
	trigL = new GuiTrigger();
	trigL->SetButtonOnlyTrigger(-1, WiiControls.LeftButton | ClassicControls.LeftButton << 16, GCControls.LeftButton);
	trigR = new GuiTrigger();
	trigR->SetButtonOnlyTrigger(-1, WiiControls.RightButton | ClassicControls.RightButton << 16, GCControls.RightButton);
	trigPlus = new GuiTrigger();
	trigPlus->SetButtonOnlyTrigger(-1, WiiControls.NextButton | ClassicControls.NextButton << 16, 0);
	trigMinus = new GuiTrigger();
	trigMinus->SetButtonOnlyTrigger(-1, WiiControls.PrevButton | ClassicControls.PrevButton << 16, 0);
	
	dialogBoxImg = new GuiImage(imgDialogBox);

	nameTxt = new GuiText("", 35, thColor("r=255 g=255 b=255 a=255 - manage windows text color"));
	if (Settings.wsprompt) nameTxt->SetWidescreen(Settings.Widescreen);
	nameTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	nameTxt->SetPosition(-6, 10);
	nameTxt->SetMaxWidth(500, SCROLL_HORIZONTAL);
	
	btnCloseBoxImg = new GuiImage(imgButtonBox);
	btnCloseBoxImg->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	btnCloseBoxImg->SetPosition(-18, 6);

	btnCloseImg = new GuiImage(imgClose);
	btnCloseImgOver = new GuiImage(imgCloseOver);
	btnClose = new GuiButton(btnCloseImg, btnCloseImgOver, 1, 5, 0, 0, trigA, btnSoundOver, btnSoundClick, 1);
	btnClose->SetTrigger(trigB);
	btnClose->SetTrigger(trigHome);
	btnClose->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	btnClose->SetPosition(-21, 8);

	btnLeftImg = new GuiImage(imgLeft);
	btnLeft = new GuiButton(btnLeftImg, btnLeftImg, 0, 5, 10, 0, trigA, btnSoundOver, btnSoundClick, 1);
	btnLeft->SetTrigger(trigL);
	btnLeft->SetTrigger(trigMinus);

	btnRightImg = new GuiImage(imgRight);
	btnRight = new GuiButton(btnRightImg, btnRightImg, 1, 5, -22, 0, trigA, btnSoundOver, btnSoundClick, 1);
	btnRight->SetTrigger(trigR);
	btnRight->SetTrigger(trigPlus);
	
	actionsButtons = new ManageButtons(0,
									   0,
									   (Settings.GodMode || !(Settings.ParentalBlocks & BLOCK_MANAGE)) ? tr( "Install" ) : NULL,
									   (Settings.GodMode || !(Settings.ParentalBlocks & BLOCK_MANAGE)) ? tr( "Delete" ) : NULL,
									   (Settings.GodMode || !(Settings.ParentalBlocks & BLOCK_MANAGE)) ? tr( "Move" ) : NULL,
									   (Settings.GodMode || !(Settings.ParentalBlocks & BLOCK_MANAGE)) ? tr( "Copy" ) : NULL);
	actionsButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	actionsButtons->SetPosition(-6, -30);
	
	installButtons = new ManageButtons(tr( "Install to:" ),
									   0,
									   tr( "Wii" ),
									   (!Settings.Sneek && Settings.EmuNand) ? tr( "Emu" ) : NULL,
									   NULL,
									   tr( "Cancel" ));
	installButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	installButtons->SetPosition(-6, -30);
	
	confirmButtons = new ManageButtons(tr( "Install from:" ), 0, tr( "OK" ), tr( "Cancel" ));
	confirmButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	confirmButtons->SetPosition(-6, -30);
	
	succesButtons = new ManageButtons(tr( "Succesfully installed" ), 0, tr( "OK" ));
	succesButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	succesButtons->SetPosition(-6, -30);
	
	errorButtons = new ManageButtons(tr( "Error" ), 0, tr( "OK" ));
	errorButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	errorButtons->SetPosition(-6, -30);
	
	cancelButtons = new ManageButtons(tr( "Install cancelled" ), 0, tr( "OK" ));
	cancelButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	cancelButtons->SetPosition(-6, -30);
	
	overWriteButtons = new ManageButtons(tr( "is already installed." ), 0, tr( "OverWrite" ), tr( "Duplicate" ), tr( "Cancel"));
	overWriteButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	overWriteButtons->SetPosition(-6, -30);
	
	miiInfos = new MiiInfos(miiSelected, BROWSE_SDUSB);
	miiInfos->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	miiInfos->SetPosition(-6, 65);
	
	Append(dialogBoxImg);
	Append(btnCloseBoxImg);
	Append(btnClose);
	Append(btnLeft);
	Append(btnRight);
	Append(nameTxt);
	Append(miiInfos);
	Append(actionsButtons);
	
	ChangeMii(NONE);
	
	SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
}

FileMiiWindow::~FileMiiWindow()
{
	StopEffect();
	SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	ResumeGui();

	while(parentElement && this->GetEffect() > 0) usleep(100);

	HaltGui();

	if(parentElement)
		((GuiWindow * ) parentElement)->Remove(this);

	RemoveAll();

	delete trigA;
	delete trigB;
	delete trigL;
	delete trigR;
	delete trigPlus;
	delete trigMinus;
	delete trigHome;

	delete imgDialogBox;
	delete imgButtonBox;
	delete imgClose;
	delete imgCloseOver;
	delete imgLeft;
	delete imgRight;

	delete dialogBoxImg;
	delete btnCloseBoxImg;
	delete btnCloseImg;
	delete btnCloseImgOver;
	delete btnLeftImg;
	delete btnRightImg;
	
	delete nameTxt;
	
	delete btnClose;
	delete btnLeft;
	delete btnRight;
	
	delete miiInfos;

	delete actionsButtons;
	delete installButtons;
	delete confirmButtons;
	delete succesButtons;
	delete errorButtons;
	delete cancelButtons;
	delete overWriteButtons;
	
	ResumeGui();
	
	mainWindow->SetState(STATE_DEFAULT);
}

void FileMiiWindow::SetWindowEffect(int direction, int in_out)
{
	if(direction == LEFT) {
		if(in_out == IN)
			SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_IN, 50);
		else
			SetEffect(EFFECT_SLIDE_LEFT | EFFECT_SLIDE_OUT, 50);
	}
	else if(direction == RIGHT)
	{
		if(in_out == IN)
			SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_IN, 50);
		else
			SetEffect(EFFECT_SLIDE_RIGHT | EFFECT_SLIDE_OUT, 50);
	}
	
	ResumeGui();
	
	while(parentElement && (this->GetEffect() > 0 || nameTxt->GetEffect() > 0)) usleep(100);
}

void FileMiiWindow::ResetButtons()
{
	if(actionsButtons)
		Remove(actionsButtons);
	if(installButtons)
		Remove(installButtons);
	if(confirmButtons)
		Remove(confirmButtons);
	if(succesButtons)
		Remove(succesButtons);
	if(errorButtons)
		Remove(errorButtons);
	if(cancelButtons)
		Remove(cancelButtons);
	if(overWriteButtons)
		Remove(overWriteButtons);
	if(miiInfos)
		Remove(miiInfos);
	
	delete actionsButtons;
	delete installButtons;
	delete confirmButtons;
	delete succesButtons;
	delete errorButtons;
	delete cancelButtons;
	delete overWriteButtons;
	delete miiInfos;
	
	actionsButtons = new ManageButtons(0,
									   0,
									   (Settings.GodMode || !(Settings.ParentalBlocks & BLOCK_MANAGE)) ? tr( "Install" ) : NULL,
									   (Settings.GodMode || !(Settings.ParentalBlocks & BLOCK_MANAGE)) ? tr( "Delete" ) : NULL,
									   (Settings.GodMode || !(Settings.ParentalBlocks & BLOCK_MANAGE)) ? tr( "Move" ) : NULL,
									   (Settings.GodMode || !(Settings.ParentalBlocks & BLOCK_MANAGE)) ? tr( "Copy" ) : NULL);
	actionsButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	actionsButtons->SetPosition(-6, -30);
	
	installButtons = new ManageButtons(tr( "Install to:" ),
									   0,
									   tr( "Wii" ),
									   (!Settings.Sneek && Settings.EmuNand) ? tr( "Emu" ) : NULL,
									   NULL,
									   tr( "Cancel" ));
	installButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	installButtons->SetPosition(-6, -30);
	
	confirmButtons = new ManageButtons(tr( "Install from:" ), 0, tr( "OK" ), tr( "Cancel" ));
	confirmButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	confirmButtons->SetPosition(-6, -30);
	
	succesButtons = new ManageButtons(tr( "Succesfully installed" ), 0, tr( "OK" ));
	succesButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	succesButtons->SetPosition(-6, -30);
	
	errorButtons = new ManageButtons(tr( "Error" ), 0, tr( "OK" ));
	errorButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	errorButtons->SetPosition(-6, -30);
	
	cancelButtons = new ManageButtons(tr( "Install cancelled" ), 0, tr( "OK" ));
	cancelButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	cancelButtons->SetPosition(-6, -30);
	
	overWriteButtons = new ManageButtons(tr( "is already installed." ), 0, tr( "OverWrite" ), tr( "Duplicate" ), tr( "Cancel"));
	overWriteButtons->SetAlignment(ALIGN_CENTRE, ALIGN_BOTTOM);
	overWriteButtons->SetPosition(-6, -30);
	
	miiInfos = new MiiInfos(miiSelected, BROWSE_SDUSB);
	miiInfos->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	miiInfos->SetPosition(-6, 65);
	
	Append(actionsButtons);
	Append(miiInfos);
}
void FileMiiWindow::RemoveEntry()
{
	FileList.RemoveEntry(miiSelected);
	
	miiSelected--;
	if (miiSelected < 0)
		miiSelected = FileList.GetFilteredCount()-1;
	
	int i = 0;
	while(!FileList.GetIsMii(miiSelected) && (i < FileList.GetFilteredCount()))
	{
		i++;
		miiSelected--;
		if (miiSelected < 0)
			miiSelected = FileList.GetFilteredCount()-1;
	}
	
	if (i >= FileList.GetFilteredCount())
	{
		mainWindow->SetState(STATE_DEFAULT);
		returnVal = 0;
		return;
	}
	
	ChangeMii(RIGHT);
}

void FileMiiWindow::ChangeMii(int EffectDirection)
{
	SetWindowEffect(EffectDirection, OUT);
	
	HaltGui();
	
	nameTxt->SetText(FileList.GetDisplayname(miiSelected).c_str());
	
	ResetButtons();
	
	EffectDirection = ((EffectDirection == LEFT) ? RIGHT : ((EffectDirection == RIGHT) ? LEFT : NONE));
	SetWindowEffect(EffectDirection, IN);
}

void FileMiiWindow::SetButtonsClickable(bool clickable)
{
	if ( !clickable )
	{
		btnClose->SetState(STATE_DISABLED);
		btnLeft->SetState(STATE_DISABLED);
		btnRight->SetState(STATE_DISABLED);
	}
	else
	{
		btnClose->SetState(STATE_DEFAULT);
		btnLeft->SetState(STATE_DEFAULT);
		btnRight->SetState(STATE_DEFAULT);
	}
}

int FileMiiWindow::Show()
{
	int choice = -1;

	while(choice == -1)
	{
		usleep(100);
		
		choice = MainLoop();
	}

	return choice;
}

int FileMiiWindow::MainLoop()
{
	if (btnClose->GetState() == STATE_CLICKED) //! close
		returnVal = 0;

	else if (btnRight->GetState() == STATE_CLICKED) //! next mii
	{
		miiSelected++;
		if (miiSelected >= FileList.GetFilteredCount())
			miiSelected = 0;
		
		int i = 0;
		while(!FileList.GetIsMii(miiSelected) && (i < FileList.GetFilteredCount()))
		{
			i++;
			miiSelected++;
			if (miiSelected >= FileList.GetFilteredCount())
				miiSelected = 0;
		}
		
		ChangeMii(LEFT);
		
		btnRight->ResetState();
	}

	else if (btnLeft->GetState() == STATE_CLICKED) //! previous mii
	{
		miiSelected--;
		if (miiSelected < 0)
			miiSelected = FileList.GetFilteredCount()-1;
		
		int i = 0;
		while(!FileList.GetIsMii(miiSelected) && (i < FileList.GetFilteredCount()))
		{
			i++;
			miiSelected--;
			if (miiSelected < 0)
				miiSelected = FileList.GetFilteredCount()-1;
		}
		
		ChangeMii(RIGHT);
		
		btnLeft->ResetState();
	}
	
	else if (actionsButtons->GetState() == STATE_CLICKED)
	{
		SetButtonsClickable(false);
		
		action = actionsButtons->GetChoice();
		
		actionsButtons->ResetState();
		actionsButtons->SetEffect(EFFECT_FADE, -20);
		
		while (actionsButtons->GetEffect()) usleep(50);
		
		HaltGui();
		
		Remove(actionsButtons);
		
		if (action == MOVE_MII || action == COPY_MII)
		{
			this->SetState(STATE_DISABLED);
			
			int res = selectBrowser(Settings.BrowserPath.c_str(), PATH);
			if( res == 2)
			{
				if (strcmp(Settings.BrowserPath.c_str(), Settings.TmpPath.c_str()))
				{
					if (Settings.TmpPath[Settings.TmpPath.size()-1] != '/')
						Settings.TmpPath += "/";
					
					if (action == MOVE_MII) confirmButtons->SetTitle(tr( "Move to:" ));
					else if (action == COPY_MII) confirmButtons->SetTitle(tr( "Copy to:" ));
					confirmButtons->SetMessage(Settings.TmpPath.c_str());
					confirmButtons->SetEffect(EFFECT_FADE, 20);
					
					Append(confirmButtons);
					ResumeGui();
					
					while (confirmButtons->GetEffect()) usleep(50);
				}
				else
				{
					errorButtons->SetMessage(tr("Source and destination folders must be different"));
					errorButtons->SetEffect(EFFECT_FADE, 20);
					
					Append(errorButtons);
					ResumeGui();
					
					while (errorButtons->GetEffect()) usleep(50);
				}
				this->SetState(STATE_DEFAULT);
				SetButtonsClickable(false);
			}
			else
			{
				actionsButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(actionsButtons);
				ResumeGui();
				
				while (actionsButtons->GetEffect()) usleep(50);
				
				this->SetState(STATE_DEFAULT);
				SetButtonsClickable(true);
			}
		}
		else if (action == DELETE_MII)
		{
			confirmButtons->SetTitle(tr( "Delete from:" ));
			confirmButtons->SetMessage(Settings.BrowserPath.c_str());
			confirmButtons->SetEffect(EFFECT_FADE, 20);
			
			Append(confirmButtons);
			
			ResumeGui();
			
			while (confirmButtons->GetEffect()) usleep(50);
		}
		else if (action == INSTALL_MII)
		{
			confirmButtons->SetTitle(tr( "Install to:" ));
			
			if(!Settings.Sneek && Settings.EmuNand)
			{
				installButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(installButtons);
				
				ResumeGui();
				
				while (installButtons->GetEffect()) usleep(50);
			}
			else
			{
				install_type = NAND_MII_INSTALL;
				
				confirmButtons->SetMessage(tr( "Wii" ));
				confirmButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(confirmButtons);
				
				ResumeGui();
				
				while (confirmButtons->GetEffect()) usleep(50);
			}
		}
	}
	
	else if (installButtons->GetState() == STATE_CLICKED)
	{
		install_type = installButtons->GetChoice();
		
		installButtons->SetEffect(EFFECT_FADE, -20);
		
		while (installButtons->GetEffect()) usleep(50);
		
		installButtons->ResetState();
		HaltGui();
		Remove(installButtons);
		
		if (install_type == NAND_MII_INSTALL) //! nand
		{
			confirmButtons->SetMessage(tr( "Wii" ));
			confirmButtons->SetEffect(EFFECT_FADE, 20);
			
			Append(confirmButtons);
			
			ResumeGui();
			
			while (confirmButtons->GetEffect()) usleep(50);
		}
		else if (install_type == EMU_MII_INSTALL) //! emu
		{
			confirmButtons->SetMessage(Settings.EmuNandPath);
			confirmButtons->SetEffect(EFFECT_FADE, 20);
			
			Append(confirmButtons);
			
			ResumeGui();
			
			while (confirmButtons->GetEffect()) usleep(50);
		}
		else //! Cancel
		{
			actionsButtons->SetEffect(EFFECT_FADE, 20);
			
			Append(actionsButtons);
			ResumeGui();
			
			while (actionsButtons->GetEffect()) usleep(50);
			
			SetButtonsClickable(true);
		}
	}
	
	else if (confirmButtons->GetState() == STATE_CLICKED)
	{
		int choice = confirmButtons->GetChoice();
		
		confirmButtons->SetEffect(EFFECT_FADE, -20);
		
		while (confirmButtons->GetEffect()) usleep(50);
		
		confirmButtons->ResetState();
		HaltGui();
		Remove(confirmButtons);
		
		if (choice == 1 && action == INSTALL_MII)
		{
			Tools = new MiiManageTools(tr( "Installing..." ));
			
			//! Generate device path
			std::string miiPath = Settings.BrowserPath;
			if (miiPath[miiPath.size()-1] != '/')
				miiPath += "/";
			miiPath += FileList.GetFilename(miiSelected);
			
			//! Check if same mii already installed in wii
			int choice = 0;
			int position = 0; //! position to the place of mii in wii file
			bool overwriteOk = false;
			
			int miiListCount = 0;
			if(install_type == NAND_MII_INSTALL)
				miiListCount = MiiList.GetFullCount();
			else
				miiListCount = EmuMiiList.GetFullCount();
			
			for(int n = 0; n < miiListCount; n++)
			{
				std::string miiName;
				if(install_type == NAND_MII_INSTALL)
					miiName = MiiList.GetFullName(n);
				else
					miiName = EmuMiiList.GetFullName(n);
				
				if(!miiName.compare(FileList.GetDisplayname(miiSelected)))
				{
					Tools->StopProgress();
					Tools->Reset();
					delete Tools;
					
					overWriteButtons->SetEffect(EFFECT_FADE, 20);
					
					Append(overWriteButtons);
					ResumeGui();
					
					while (overWriteButtons->GetEffect()) usleep(50);
					
					while (choice == 0) choice = overWriteButtons->GetChoice();
					
					overWriteButtons->SetEffect(EFFECT_FADE, -20);
					
					overWriteButtons->ResetState();
					HaltGui();
					Remove(overWriteButtons);
					
					if(choice == 1)
					{
						overwriteOk = true;
						if(install_type == NAND_MII_INSTALL)
							position = MiiList.GetFullPosition(n);
						else
							position = EmuMiiList.GetFullPosition(n);
					}
					else if(choice == 3)
					{
						actionsButtons->SetEffect(EFFECT_FADE, 20);
						Append(actionsButtons);
						ResumeGui();
						
						while (actionsButtons->GetEffect()) usleep(50);
						SetButtonsClickable(true);
						goto out;
					}
					
					Tools = new MiiManageTools(tr( "Installing..." ));
					n = miiListCount;
				}
			}
			
			if(!overwriteOk)
			{
				if(install_type == NAND_MII_INSTALL)
					position = Tools->FindFirstFree(true);
				else
					position = Tools->FindFirstFree(false);
			}
			if(install_type == NAND_MII_INSTALL)
				Tools->InstallMii_Dev_Nand(miiPath, position);
			else
				Tools->InstallMii_Dev_Emu(miiPath, position);
			
			while (Tools->GetResult() == 0) usleep(50);
			
			if(install_type == NAND_MII_INSTALL)
				MiiList.SetRefresh();
			else
				EmuMiiList.SetRefresh();
			
			Tools->StopProgress();
			
			if (Tools->GetResult() > 0)
			{
				succesButtons->SetTitle(tr( "Succesfully installed" ));
				succesButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(succesButtons);
				ResumeGui();
				
				while (succesButtons->GetEffect()) usleep(50);
			}
			else if (Tools->GetResult() == -10)
			{
				cancelButtons->SetTitle(tr( "Install cancelled" ));
				cancelButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(cancelButtons);
				ResumeGui();
				
				while (cancelButtons->GetEffect()) usleep(50);
			}
			else if (Tools->GetResult() < 0 && Tools->GetResult() != -10)
			{
				errorButtons->SetMessage(tr("Unable to copy datas"));
				errorButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(errorButtons);
				ResumeGui();
				
				while (errorButtons->GetEffect()) usleep(50);
			}
			
			Tools->Reset();
			delete Tools;
			
		  out: usleep(1);
		}
		else if (choice == 1 && action == DELETE_MII)
		{
			Tools = new MiiManageTools(tr("Deleting..."));
			
			std::string miiPath = Settings.BrowserPath;
			if (miiPath[miiPath.size()-1] != '/')
				miiPath += "/";
			miiPath += FileList.GetFilename(miiSelected);
			
			Tools->DeleteMii_Dev(miiPath);
			
			while (Tools->GetResult() == 0) usleep(50);
			
			Tools->StopProgress();
			
			if (Tools->GetResult() > 0)
			{
				refreshNeeded = true;
				
				succesButtons->SetTitle(tr( "Succesfully deleted" ));
				succesButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(succesButtons);
				ResumeGui();
				
				while (succesButtons->GetEffect()) usleep(50);
			}
			else if (Tools->GetResult() == -10)
			{
				cancelButtons->SetTitle(tr( "Delete cancelled" ));
				cancelButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(cancelButtons);
				ResumeGui();
				
				while (cancelButtons->GetEffect()) usleep(50);
			}
			else if (Tools->GetResult() < 0 && Tools->GetResult() != -10)
			{
				errorButtons->SetMessage(tr("Unable to delete datas"));
				errorButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(errorButtons);
				ResumeGui();
				
				while (errorButtons->GetEffect()) usleep(50);
			}
			
			Tools->Reset();
			delete Tools;
		}
		else if (choice == 1 && action == MOVE_MII)
		{
			Tools = new MiiManageTools(tr("Moving..."));
			
			std::string srcPath = Settings.BrowserPath;
			if (srcPath[srcPath.size()-1] != '/')
				srcPath += "/";
			srcPath += FileList.GetFilename(miiSelected);
			Settings.TmpPath += FileList.GetFilename(miiSelected);
			
			Tools->MoveMii(srcPath, Settings.TmpPath);
			
			while (Tools->GetResult() == 0) usleep(50);
			
			Tools->StopProgress();
			
			if (Tools->GetResult() > 0)
			{
				refreshNeeded = true;
				
				succesButtons->SetTitle(tr( "Succesfully moved" ));
				succesButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(succesButtons);
				ResumeGui();
				
				while (succesButtons->GetEffect()) usleep(50);
			}
			else if (Tools->GetResult() == -10)
			{
				cancelButtons->SetTitle(tr( "Move cancelled" ));
				cancelButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(cancelButtons);
				ResumeGui();
				
				while (cancelButtons->GetEffect()) usleep(50);
			}
			else if (Tools->GetResult() < 0 && Tools->GetResult() != -10)
			{
				errorButtons->SetMessage(tr("Unable to move datas"));
				errorButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(errorButtons);
				ResumeGui();
				
				while (errorButtons->GetEffect()) usleep(50);
			}
			
			Tools->Reset();
			delete Tools;
		}
		else if (choice == 1 && action == COPY_MII)
		{
			Tools = new MiiManageTools(tr("Copying..."));
			
			std::string srcPath = Settings.BrowserPath;
			if (srcPath[srcPath.size()-1] != '/')
				srcPath += "/";
			srcPath += FileList.GetFilename(miiSelected);
			Settings.TmpPath += FileList.GetFilename(miiSelected);
			
			Tools->CopyMii(srcPath, Settings.TmpPath);
			
			while (Tools->GetResult() == 0) usleep(50);
			
			Tools->StopProgress();
			
			if (Tools->GetResult() > 0)
			{
				succesButtons->SetTitle(tr( "Succesfully copied" ));
				succesButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(succesButtons);
				ResumeGui();
				
				while (succesButtons->GetEffect()) usleep(50);
			}
			else if (Tools->GetResult() == -10)
			{
				cancelButtons->SetTitle(tr( "Copy cancelled" ));
				cancelButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(cancelButtons);
				ResumeGui();
				
				while (cancelButtons->GetEffect()) usleep(50);
			}
			else if (Tools->GetResult() < 0 && Tools->GetResult() != -10)
			{
				errorButtons->SetMessage(tr("Unable to copy datas"));
				errorButtons->SetEffect(EFFECT_FADE, 20);
				
				Append(errorButtons);
				ResumeGui();
				
				while (errorButtons->GetEffect()) usleep(50);
			}
			
			Tools->Reset();
			delete Tools;
		}
		else //! Cancel
		{
			actionsButtons->SetEffect(EFFECT_FADE, 20);
			Append(actionsButtons);
			ResumeGui();
			
			while (actionsButtons->GetEffect()) usleep(50);
			
			SetButtonsClickable(true);
		}
	}
	
	else if (succesButtons->GetState() == STATE_CLICKED)
	{
		if (!refreshNeeded)
		{
			succesButtons->SetEffect(EFFECT_FADE, -20);
			actionsButtons->SetEffect(EFFECT_FADE, 20);
			
			while (succesButtons->GetEffect()) usleep(50);
			
			succesButtons->ResetState();
			
			HaltGui();
			
			Remove(succesButtons);
			Append(actionsButtons);
			
			ResumeGui();
			
			while (actionsButtons->GetEffect()) usleep(50);
		}
		else
			RemoveEntry();
		
		SetButtonsClickable(true);
	}
	
	else if (errorButtons->GetState() == STATE_CLICKED)
	{
		errorButtons->SetEffect(EFFECT_FADE, -20);
		actionsButtons->SetEffect(EFFECT_FADE, 20);
		
		while (errorButtons->GetEffect()) usleep(50);
		
		errorButtons->ResetState();
		
		HaltGui();
		
		Remove(errorButtons);
		Append(actionsButtons);
		
		ResumeGui();
		
		while (actionsButtons->GetEffect()) usleep(50);
		
		SetButtonsClickable(true);
	}
	
	else if (cancelButtons->GetState() == STATE_CLICKED)
	{
		cancelButtons->SetEffect(EFFECT_FADE, -20);
		actionsButtons->SetEffect(EFFECT_FADE, 20);
		
		while (cancelButtons->GetEffect()) usleep(50);
		
		cancelButtons->ResetState();
		
		HaltGui();
		
		Remove(cancelButtons);
		Append(actionsButtons);
		
		ResumeGui();
		
		while (actionsButtons->GetEffect()) usleep(50);
		
		SetButtonsClickable(true);
	}
	
	return returnVal;
}
