
package recognition;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Vector;
import javax.microedition.io.Connector;
import javax.microedition.io.SocketConnection;
import javax.microedition.io.file.FileConnection;
import javax.microedition.midlet.*;
import javax.microedition.lcdui.*;
import org.netbeans.microedition.lcdui.WaitScreen;
import org.netbeans.microedition.lcdui.pda.FileBrowser;
import org.netbeans.microedition.util.SimpleCancellableTask;

/**
 * @author Octavian Sima
 *
 * Face Recognition mobile phone application
 * Browse for a image file on phone memory, send it to server and
 * receive results (recognition persons image and info)
 */
public class FaceRecognition extends MIDlet implements CommandListener {
    //application settings
    private final int PACKAGE_MAX_SIZE = 10000;
    private final boolean USE_ACK = true;   //send/receive ACK after each package
    private final boolean DEBUG_ON = true;
    
    private String serverAddress = "127.0.0.1";
    private String serverPort = "10000";
    //candidates number for each detected face
    private String candidatesNumber = "1";

    //file connection for image file that will be sent to server
    private FileConnection fileCon = null;
    //image file size
    private int imageSize = 0;
    //display size
    private int screenWidth, screenHeight;
    //next displayable after alert
    private Displayable nextDisplayable = null;
    private boolean midletPaused = false;
    
    //<editor-fold defaultstate="collapsed" desc=" Generated Fields ">//GEN-BEGIN:|fields|0|
    private Command exitCommand;
    private Command selectImageCommand;
    private Command recognizeCommand;
    private Command cancelCommand;
    private Command settingsCommand;
    private Command cancelSettingsCommand;
    private Command saveSettingsCommand;
    private Command cancelCommand1;
    private Command newRecognitionCommand;
    private Command helpCommand;
    private Command backCommand;
    private Command backCommand1;
    private Form mainForm;
    private FileBrowser fileBrowser;
    private Form selectedImageForm;
    private WaitScreen transferWaitScreen;
    private Form settingsForm;
    private TextField candidatesNoTextField;
    private TextField serverAddressTextField;
    private TextField serverPortTextField;
    private Form resultForm;
    private Form helpForm;
    private StringItem stringItem;
    private StringItem stringItem1;
    private StringItem stringItem2;
    private Image waitImage;
    private SimpleCancellableTask task;
    private Image backgroundImage;
    //</editor-fold>//GEN-END:|fields|0|

    /**
     * The FaceRecognition constructor.
     */
    public FaceRecognition() {
    }

    //<editor-fold defaultstate="collapsed" desc=" Generated Methods ">//GEN-BEGIN:|methods|0|
    //</editor-fold>//GEN-END:|methods|0|

    //<editor-fold defaultstate="collapsed" desc=" Generated Method: initialize ">//GEN-BEGIN:|0-initialize|0|0-preInitialize
    /**
     * Initilizes the application.
     * It is called only once when the MIDlet is started. The method is called before the <code>startMIDlet</code> method.
     */
    private void initialize() {//GEN-END:|0-initialize|0|0-preInitialize
        // write pre-initialize user code here
//GEN-LINE:|0-initialize|1|0-postInitialize
        // write post-initialize user code here
        Canvas dummyCanvas = new Canvas() {
            protected void paint(Graphics g) {
                throw new UnsupportedOperationException("Not supported yet.");
            }
        };
        // get the dimensions of the screen
        screenWidth = dummyCanvas.getWidth ();
        screenHeight = dummyCanvas.getHeight();

    }//GEN-BEGIN:|0-initialize|2|
    //</editor-fold>//GEN-END:|0-initialize|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Method: startMIDlet ">//GEN-BEGIN:|3-startMIDlet|0|3-preAction
    /**
     * Performs an action assigned to the Mobile Device - MIDlet Started point.
     */
    public void startMIDlet() {//GEN-END:|3-startMIDlet|0|3-preAction

        switchDisplayable(null, getMainForm());//GEN-LINE:|3-startMIDlet|1|3-postAction

    }//GEN-BEGIN:|3-startMIDlet|2|
    //</editor-fold>//GEN-END:|3-startMIDlet|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Method: resumeMIDlet ">//GEN-BEGIN:|4-resumeMIDlet|0|4-preAction
    /**
     * Performs an action assigned to the Mobile Device - MIDlet Resumed point.
     */
    public void resumeMIDlet() {//GEN-END:|4-resumeMIDlet|0|4-preAction
        // write pre-action user code here
//GEN-LINE:|4-resumeMIDlet|1|4-postAction
        // write post-action user code here
    }//GEN-BEGIN:|4-resumeMIDlet|2|
    //</editor-fold>//GEN-END:|4-resumeMIDlet|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Method: switchDisplayable ">//GEN-BEGIN:|5-switchDisplayable|0|5-preSwitch
    /**
     * Switches a current displayable in a display. The <code>display</code> instance is taken from <code>getDisplay</code> method. This method is used by all actions in the design for switching displayable.
     * @param alert the Alert which is temporarily set to the display; if <code>null</code>, then <code>nextDisplayable</code> is set immediately
     * @param nextDisplayable the Displayable to be set
     */
    public void switchDisplayable(Alert alert, Displayable nextDisplayable) {//GEN-END:|5-switchDisplayable|0|5-preSwitch

        Display display = getDisplay();//GEN-BEGIN:|5-switchDisplayable|1|5-postSwitch
        if (alert == null) {
            display.setCurrent(nextDisplayable);
        } else {
            display.setCurrent(alert, nextDisplayable);
        }//GEN-END:|5-switchDisplayable|1|5-postSwitch

    }//GEN-BEGIN:|5-switchDisplayable|2|
    //</editor-fold>//GEN-END:|5-switchDisplayable|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Method: commandAction for Displayables ">//GEN-BEGIN:|7-commandAction|0|7-preCommandAction
    /**
     * Called by a system to indicated that a command has been invoked on a particular displayable.
     * @param command the Command that was invoked
     * @param displayable the Displayable where the command was invoked
     */
    public void commandAction(Command command, Displayable displayable) {//GEN-END:|7-commandAction|0|7-preCommandAction

        if (displayable == fileBrowser) {//GEN-BEGIN:|7-commandAction|1|24-preAction
            if (command == FileBrowser.SELECT_FILE_COMMAND) {//GEN-END:|7-commandAction|1|24-preAction

                if (selectedImageForm != null) {
                    selectedImageForm.deleteAll();
                    selectedImageForm = null;
                }

                try {
                    fileCon = fileBrowser.getSelectedFile();
                    if (!fileCon.exists()) {
                        throw new IOException("File does not exists");
                    }
                    if (!fileCon.canRead()) {
                        throw new IOException("File can't be read");
                    }
                    imageSize = (int)fileCon.fileSize();
                } catch (IOException ex) {
                    ex.printStackTrace();
                }

                switchDisplayable(null, getSelectedImageForm());//GEN-LINE:|7-commandAction|2|24-postAction

            } else if (command == exitCommand) {//GEN-LINE:|7-commandAction|3|36-preAction

                exitMIDlet();//GEN-LINE:|7-commandAction|4|36-postAction

            }//GEN-BEGIN:|7-commandAction|5|122-preAction
        } else if (displayable == helpForm) {
            if (command == backCommand1) {//GEN-END:|7-commandAction|5|122-preAction
                // write pre-action user code here
                switchDisplayable(null, getMainForm());//GEN-LINE:|7-commandAction|6|122-postAction
                // write post-action user code here
            }//GEN-BEGIN:|7-commandAction|7|19-preAction
        } else if (displayable == mainForm) {
            if (command == exitCommand) {//GEN-END:|7-commandAction|7|19-preAction

                exitMIDlet();//GEN-LINE:|7-commandAction|8|19-postAction

            } else if (command == helpCommand) {//GEN-LINE:|7-commandAction|9|118-preAction
                // write pre-action user code here
                switchDisplayable(null, getHelpForm());//GEN-LINE:|7-commandAction|10|118-postAction
                // write post-action user code here
            } else if (command == selectImageCommand) {//GEN-LINE:|7-commandAction|11|26-preAction

                switchDisplayable(null, getFileBrowser());//GEN-LINE:|7-commandAction|12|26-postAction

            } else if (command == settingsCommand) {//GEN-LINE:|7-commandAction|13|86-preAction

                switchDisplayable(null, getSettingsForm());//GEN-LINE:|7-commandAction|14|86-postAction

            }//GEN-BEGIN:|7-commandAction|15|104-preAction
        } else if (displayable == resultForm) {
            if (command == exitCommand) {//GEN-END:|7-commandAction|15|104-preAction
                // write pre-action user code here
                exitMIDlet();//GEN-LINE:|7-commandAction|16|104-postAction
                // write post-action user code here
            } else if (command == newRecognitionCommand) {//GEN-LINE:|7-commandAction|17|107-preAction
                // write pre-action user code here
                switchDisplayable(null, getMainForm());//GEN-LINE:|7-commandAction|18|107-postAction
                // write post-action user code here
            }//GEN-BEGIN:|7-commandAction|19|114-preAction
        } else if (displayable == selectedImageForm) {
            if (command == backCommand) {//GEN-END:|7-commandAction|19|114-preAction
                // write pre-action user code here
                switchDisplayable(null, getFileBrowser());//GEN-LINE:|7-commandAction|20|114-postAction
                // write post-action user code here
            } else if (command == recognizeCommand) {//GEN-LINE:|7-commandAction|21|41-preAction

                switchDisplayable(null, getTransferWaitScreen());//GEN-LINE:|7-commandAction|22|41-postAction

            }//GEN-BEGIN:|7-commandAction|23|94-preAction
        } else if (displayable == settingsForm) {
            if (command == cancelSettingsCommand) {//GEN-END:|7-commandAction|23|94-preAction

                switchDisplayable(null, getMainForm());//GEN-LINE:|7-commandAction|24|94-postAction

            } else if (command == saveSettingsCommand) {//GEN-LINE:|7-commandAction|25|90-preAction

                serverAddress = serverAddressTextField.getString();
                try {
                    Integer.parseInt(serverPortTextField.getString());
                    int candNo = Integer.parseInt(candidatesNoTextField.getString());
                    if (candNo > 10)
                        throw  new Exception("Invalid candidates number");

                    serverPort = serverPortTextField.getString();
                    candidatesNumber = candidatesNoTextField.getString();
                    switchDisplayable(null, mainForm);
                } catch(Exception e) {
                    Alert a = new Alert("Invalid settings",
		    "Please insert valid values!", null, AlertType.ERROR);
                    a.setTimeout(Alert.FOREVER);
                    a.setCommandListener(this);
                    nextDisplayable = settingsForm;
                    switchDisplayable(a, settingsForm);
                }
//GEN-LINE:|7-commandAction|26|90-postAction

            }//GEN-BEGIN:|7-commandAction|27|48-preAction
        } else if (displayable == transferWaitScreen) {
            if (command == WaitScreen.FAILURE_COMMAND) {//GEN-END:|7-commandAction|27|48-preAction

//GEN-LINE:|7-commandAction|28|48-postAction

            } else if (command == WaitScreen.SUCCESS_COMMAND) {//GEN-LINE:|7-commandAction|29|47-preAction
                task = null;
                switchDisplayable(null, getResultForm());//GEN-LINE:|7-commandAction|30|47-postAction

            } else if (command == cancelCommand) {//GEN-LINE:|7-commandAction|31|55-preAction

                switchDisplayable(null, getSelectedImageForm());//GEN-LINE:|7-commandAction|32|55-postAction
                transferWaitScreen.getTask().cancel();
                task = null;
                switchDisplayable(null, mainForm);
            }//GEN-BEGIN:|7-commandAction|33|7-postCommandAction
        }//GEN-END:|7-commandAction|33|7-postCommandAction
        if (command == Alert.DISMISS_COMMAND) {
            switchDisplayable(null, nextDisplayable);
        }
    }//GEN-BEGIN:|7-commandAction|34|
    //</editor-fold>//GEN-END:|7-commandAction|34|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: exitCommand ">//GEN-BEGIN:|18-getter|0|18-preInit
    /**
     * Returns an initiliazed instance of exitCommand component.
     * @return the initialized component instance
     */
    public Command getExitCommand() {
        if (exitCommand == null) {//GEN-END:|18-getter|0|18-preInit

            exitCommand = new Command("Exit", Command.EXIT, 0);//GEN-LINE:|18-getter|1|18-postInit

        }//GEN-BEGIN:|18-getter|2|
        return exitCommand;
    }
    //</editor-fold>//GEN-END:|18-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: mainForm ">//GEN-BEGIN:|14-getter|0|14-preInit
    /**
     * Returns an initiliazed instance of mainForm component.
     * @return the initialized component instance
     */
    public Form getMainForm() {
        if (mainForm == null) {//GEN-END:|14-getter|0|14-preInit

            mainForm = new Form("iKnowU", new Item[] { });//GEN-BEGIN:|14-getter|1|14-postInit
            mainForm.addCommand(getExitCommand());
            mainForm.addCommand(getSelectImageCommand());
            mainForm.addCommand(getSettingsCommand());
            mainForm.addCommand(getHelpCommand());
            mainForm.setCommandListener(this);//GEN-END:|14-getter|1|14-postInit
            mainForm.append(getBackgroundImage());

        }//GEN-BEGIN:|14-getter|2|
        return mainForm;
    }
    //</editor-fold>//GEN-END:|14-getter|2|
    
    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: selectImageCommand ">//GEN-BEGIN:|25-getter|0|25-preInit
    /**
     * Returns an initiliazed instance of selectImageCommand component.
     * @return the initialized component instance
     */
    public Command getSelectImageCommand() {
        if (selectImageCommand == null) {//GEN-END:|25-getter|0|25-preInit

            selectImageCommand = new Command("Select Photo", Command.OK, 0);//GEN-LINE:|25-getter|1|25-postInit

        }//GEN-BEGIN:|25-getter|2|
        return selectImageCommand;
    }
    //</editor-fold>//GEN-END:|25-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: fileBrowser ">//GEN-BEGIN:|22-getter|0|22-preInit
    /**
     * Returns an initiliazed instance of fileBrowser component.
     * @return the initialized component instance
     */
    public FileBrowser getFileBrowser() {
        if (fileBrowser == null) {//GEN-END:|22-getter|0|22-preInit

            fileBrowser = new FileBrowser(getDisplay());//GEN-BEGIN:|22-getter|1|22-postInit
            fileBrowser.setTitle("fileBrowser");
            fileBrowser.setCommandListener(this);
            fileBrowser.addCommand(FileBrowser.SELECT_FILE_COMMAND);
            fileBrowser.addCommand(getExitCommand());//GEN-END:|22-getter|1|22-postInit

        }//GEN-BEGIN:|22-getter|2|
        return fileBrowser;
    }
    //</editor-fold>//GEN-END:|22-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: selectedImageForm ">//GEN-BEGIN:|28-getter|0|28-preInit
    /**
     * Returns an initiliazed instance of selectedImageForm component.
     * @return the initialized component instance
     */
    public Form getSelectedImageForm() {
        if (selectedImageForm == null) {//GEN-END:|28-getter|0|28-preInit

            selectedImageForm = new Form("Selected Image", new Item[] { });//GEN-BEGIN:|28-getter|1|28-postInit
            selectedImageForm.addCommand(getBackCommand());
            selectedImageForm.addCommand(getRecognizeCommand());
            selectedImageForm.setCommandListener(this);//GEN-END:|28-getter|1|28-postInit
            Image scaledImage = null;
            try {
                InputStream imageInputStream = fileCon.openInputStream();
                scaledImage = resizeImage(Image.createImage(imageInputStream),
                       screenWidth, screenHeight-20);
                imageInputStream.close();
            } catch (IOException ex) {
                ex.printStackTrace();
            }
            selectedImageForm.append(scaledImage);
        }//GEN-BEGIN:|28-getter|2|
        return selectedImageForm;
    }
    //</editor-fold>//GEN-END:|28-getter|2|



    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: recognizeCommand ">//GEN-BEGIN:|40-getter|0|40-preInit
    /**
     * Returns an initiliazed instance of recognizeCommand component.
     * @return the initialized component instance
     */
    public Command getRecognizeCommand() {
        if (recognizeCommand == null) {//GEN-END:|40-getter|0|40-preInit

            recognizeCommand = new Command("Recognize", Command.OK, 0);//GEN-LINE:|40-getter|1|40-postInit

        }//GEN-BEGIN:|40-getter|2|
        return recognizeCommand;
    }
    //</editor-fold>//GEN-END:|40-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: transferWaitScreen ">//GEN-BEGIN:|44-getter|0|44-preInit
    /**
     * Returns an initiliazed instance of transferWaitScreen component.
     * @return the initialized component instance
     */
    public WaitScreen getTransferWaitScreen() {
        if (transferWaitScreen == null) {//GEN-END:|44-getter|0|44-preInit

            transferWaitScreen = new WaitScreen(getDisplay());//GEN-BEGIN:|44-getter|1|44-postInit
            transferWaitScreen.setTitle("Recognizing...");
            transferWaitScreen.addCommand(getCancelCommand());
            transferWaitScreen.setCommandListener(this);
            transferWaitScreen.setImage(getWaitImage());
            transferWaitScreen.setText("Waiting for image transfer...\n");
            transferWaitScreen.setTask(getTask());//GEN-END:|44-getter|1|44-postInit

        }//GEN-BEGIN:|44-getter|2|
        return transferWaitScreen;
    }
    //</editor-fold>//GEN-END:|44-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: task ">//GEN-BEGIN:|49-getter|0|49-preInit
    /**
     * Returns an initiliazed instance of task component.
     * @return the initialized component instance
     */
    public SimpleCancellableTask getTask() {
        if (task == null) {//GEN-END:|49-getter|0|49-preInit

            task = new SimpleCancellableTask();//GEN-BEGIN:|49-getter|1|49-execute
            task.setExecutable(new org.netbeans.microedition.util.Executable() {
                public void execute() throws Exception {//GEN-END:|49-getter|1|49-execute

                    InputStream imageInputStream = fileCon.openInputStream();
                    transferWaitScreen.setText("Connecting to server on port " + serverPort + "...");
                    SocketConnection sc = (SocketConnection) Connector.open("socket://" +
                            serverAddress + ":" + serverPort);

                    DataInputStream dis = sc.openDataInputStream();
                    DataOutputStream dos = sc.openDataOutputStream();

                    transferWaitScreen.setText("Sending candidates number (" + candidatesNumber + ") ...");
                    dos.write(Integer.parseInt(candidatesNumber));
                    dos.flush();

                    transferWaitScreen.setText("Sending image file ...");

                    sendImage(dos, dis, imageInputStream, imageSize);
                    imageInputStream.close();

                    //clean result form
                    getResultForm().deleteAll();

                    //get recognition result
                    //get detected persons number
                    int detectedPersonsNo = dis.readInt();
                    if (USE_ACK) {
                        sendACK(dos);
                        receiveACK(dis);
                    }
                    if (detectedPersonsNo == 0)
                        getResultForm().append("No detected person.\n");

                    for (int i = 1; i <= detectedPersonsNo; i++) {
                        getResultForm().append("Detected Person " + i+":\n");
                        //receive detected face image
                        transferWaitScreen.setText("Receving detected face image file ...");
                        byte[] detImageByteBuff = receiveImage(dos, dis);
                        Image detImg = Image.createImage(detImageByteBuff, 0, detImageByteBuff.length);
                        getResultForm().append(detImg);

                        //get candidates number for curent detected face
                        int candidates = dis.readInt();
                        if (USE_ACK) {
                            sendACK(dos);
                            receiveACK(dis);
                        }

                        if (candidates == 0) {
                            getResultForm().append("\nUnknown person.\n");
                        } else {
                            getResultForm().append("\nCandidates :\n");

                            //receive candidates info
                            for (int j = 0; j < candidates; j++) {
                                //receive face image
                                transferWaitScreen.setText("Receving image file ...");
                                byte[] imageByteBuff = receiveImage(dos, dis);
                                Image img = Image.createImage(imageByteBuff, 0, imageByteBuff.length);
                                getResultForm().append(img);

                                //receive recognized person info
                                int infoSize = dis.readInt();
                                if (USE_ACK) {
                                    sendACK(dos);
                                    receiveACK(dis);
                                }
                                byte[] infoBuff = new byte[infoSize];
                                receiveBytes(dos, dis, infoBuff, infoSize);
                                String[] tokens = split(new String(infoBuff), "#");
                                getResultForm().append(new StringItem("Name:", tokens[0]));
                                getResultForm().append(new StringItem("Phone:", tokens[1]));
                                getResultForm().append(new StringItem("BirthDate:", tokens[2]));
                                getResultForm().append(new StringItem("Ocupation:", tokens[3]));
                                getResultForm().append(new StringItem("Recogn. Confidence:", tokens[4]));
                                getResultForm().append("\n");
                            }
                        }

                    }
                }//GEN-BEGIN:|49-getter|2|49-postInit
            });//GEN-END:|49-getter|2|49-postInit

        }//GEN-BEGIN:|49-getter|3|
        return task;
    }
    //</editor-fold>//GEN-END:|49-getter|3|
   
    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: waitImage ">//GEN-BEGIN:|51-getter|0|51-preInit
    /**
     * Returns an initiliazed instance of waitImage component.
     * @return the initialized component instance
     */
    public Image getWaitImage() {
        if (waitImage == null) {//GEN-END:|51-getter|0|51-preInit

            try {//GEN-BEGIN:|51-getter|1|51-@java.io.IOException
                waitImage = Image.createImage("/wait_icon.gif");
            } catch (java.io.IOException e) {//GEN-END:|51-getter|1|51-@java.io.IOException
                e.printStackTrace();
            }//GEN-LINE:|51-getter|2|51-postInit

        }//GEN-BEGIN:|51-getter|3|
        return waitImage;
    }
    //</editor-fold>//GEN-END:|51-getter|3|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: cancelCommand ">//GEN-BEGIN:|54-getter|0|54-preInit
    /**
     * Returns an initiliazed instance of cancelCommand component.
     * @return the initialized component instance
     */
    public Command getCancelCommand() {
        if (cancelCommand == null) {//GEN-END:|54-getter|0|54-preInit

            cancelCommand = new Command("Cancel", Command.CANCEL, 0);//GEN-LINE:|54-getter|1|54-postInit

        }//GEN-BEGIN:|54-getter|2|
        return cancelCommand;
    }
    //</editor-fold>//GEN-END:|54-getter|2|
    

    
    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: backgroundImage ">//GEN-BEGIN:|65-getter|0|65-preInit
    /**
     * Returns an initiliazed instance of backgroundImage component.
     * @return the initialized component instance
     */
    public Image getBackgroundImage() {
        if (backgroundImage == null) {//GEN-END:|65-getter|0|65-preInit

            try {//GEN-BEGIN:|65-getter|1|65-@java.io.IOException
                backgroundImage = Image.createImage("/back.jpg");
            } catch (java.io.IOException e) {//GEN-END:|65-getter|1|65-@java.io.IOException
                e.printStackTrace();
            }//GEN-LINE:|65-getter|2|65-postInit

            backgroundImage = resizeImage(backgroundImage, screenWidth, screenHeight-20);
        }//GEN-BEGIN:|65-getter|3|
        return backgroundImage;
    }
    //</editor-fold>//GEN-END:|65-getter|3|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: settingsForm ">//GEN-BEGIN:|73-getter|0|73-preInit
    /**
     * Returns an initiliazed instance of settingsForm component.
     * @return the initialized component instance
     */
    public Form getSettingsForm() {
        if (settingsForm == null) {//GEN-END:|73-getter|0|73-preInit
            settingsForm = new Form("Server Settings", new Item[] { getServerAddressTextField(), getServerPortTextField(), getCandidatesNoTextField() });//GEN-BEGIN:|73-getter|1|73-postInit
            settingsForm.addCommand(getSaveSettingsCommand());
            settingsForm.addCommand(getCancelSettingsCommand());
            settingsForm.setCommandListener(this);//GEN-END:|73-getter|1|73-postInit

        }//GEN-BEGIN:|73-getter|2|
        return settingsForm;
    }
    //</editor-fold>//GEN-END:|73-getter|2|
   
    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: serverAddressTextField ">//GEN-BEGIN:|74-getter|0|74-preInit
    /**
     * Returns an initiliazed instance of serverAddressTextField component.
     * @return the initialized component instance
     */
    public TextField getServerAddressTextField() {
        if (serverAddressTextField == null) {//GEN-END:|74-getter|0|74-preInit

            serverAddressTextField = new TextField("Server address:", serverAddress, 32, TextField.ANY);//GEN-LINE:|74-getter|1|74-postInit

        }//GEN-BEGIN:|74-getter|2|
        return serverAddressTextField;
    }
    //</editor-fold>//GEN-END:|74-getter|2|
   
    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: serverPortTextField ">//GEN-BEGIN:|75-getter|0|75-preInit
    /**
     * Returns an initiliazed instance of serverPortTextField component.
     * @return the initialized component instance
     */
    public TextField getServerPortTextField() {
        if (serverPortTextField == null) {//GEN-END:|75-getter|0|75-preInit

            serverPortTextField = new TextField("Server port:", serverPort//GEN-BEGIN:|75-getter|1|75-postInit
                    , 32, TextField.ANY);//GEN-END:|75-getter|1|75-postInit

        }//GEN-BEGIN:|75-getter|2|
        return serverPortTextField;
    }
    //</editor-fold>//GEN-END:|75-getter|2|
   
    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: candidatesNoTextField ">//GEN-BEGIN:|76-getter|0|76-preInit
    /**
     * Returns an initiliazed instance of candidatesNoTextField component.
     * @return the initialized component instance
     */
    public TextField getCandidatesNoTextField() {
        if (candidatesNoTextField == null) {//GEN-END:|76-getter|0|76-preInit

            candidatesNoTextField = new TextField("Max matched faces (<10):", candidatesNumber, 32, TextField.ANY);//GEN-LINE:|76-getter|1|76-postInit

        }//GEN-BEGIN:|76-getter|2|
        return candidatesNoTextField;
    }
    //</editor-fold>//GEN-END:|76-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: settingsCommand ">//GEN-BEGIN:|85-getter|0|85-preInit
    /**
     * Returns an initiliazed instance of settingsCommand component.
     * @return the initialized component instance
     */
    public Command getSettingsCommand() {
        if (settingsCommand == null) {//GEN-END:|85-getter|0|85-preInit

            settingsCommand = new Command("Settings", Command.OK, 0);//GEN-LINE:|85-getter|1|85-postInit

        }//GEN-BEGIN:|85-getter|2|
        return settingsCommand;
    }
    //</editor-fold>//GEN-END:|85-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: saveSettingsCommand ">//GEN-BEGIN:|89-getter|0|89-preInit
    /**
     * Returns an initiliazed instance of saveSettingsCommand component.
     * @return the initialized component instance
     */
    public Command getSaveSettingsCommand() {
        if (saveSettingsCommand == null) {//GEN-END:|89-getter|0|89-preInit

            saveSettingsCommand = new Command("Save", Command.OK, 0);//GEN-LINE:|89-getter|1|89-postInit

        }//GEN-BEGIN:|89-getter|2|
        return saveSettingsCommand;
    }
    //</editor-fold>//GEN-END:|89-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: resultForm ">//GEN-BEGIN:|92-getter|0|92-preInit
    /**
     * Returns an initiliazed instance of resultForm component.
     * @return the initialized component instance
     */
    public Form getResultForm() {
        if (resultForm == null) {//GEN-END:|92-getter|0|92-preInit

            resultForm = new Form("Recognition Result", new Item[] { });//GEN-BEGIN:|92-getter|1|92-postInit
            resultForm.addCommand(getExitCommand());
            resultForm.addCommand(getNewRecognitionCommand());
            resultForm.setCommandListener(this);//GEN-END:|92-getter|1|92-postInit

        }//GEN-BEGIN:|92-getter|2|
        return resultForm;
    }
    //</editor-fold>//GEN-END:|92-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: cancelSettingsCommand ">//GEN-BEGIN:|93-getter|0|93-preInit
    /**
     * Returns an initiliazed instance of cancelSettingsCommand component.
     * @return the initialized component instance
     */
    public Command getCancelSettingsCommand() {
        if (cancelSettingsCommand == null) {//GEN-END:|93-getter|0|93-preInit

            cancelSettingsCommand = new Command("Cancel", Command.CANCEL, 0);//GEN-LINE:|93-getter|1|93-postInit

        }//GEN-BEGIN:|93-getter|2|
        return cancelSettingsCommand;
    }
    //</editor-fold>//GEN-END:|93-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: newRecognitionCommand ">//GEN-BEGIN:|106-getter|0|106-preInit
    /**
     * Returns an initiliazed instance of newRecognitionCommand component.
     * @return the initialized component instance
     */
    public Command getNewRecognitionCommand() {
        if (newRecognitionCommand == null) {//GEN-END:|106-getter|0|106-preInit
            // write pre-init user code here
            newRecognitionCommand = new Command("New Recognition", Command.OK, 0);//GEN-LINE:|106-getter|1|106-postInit
            // write post-init user code here
        }//GEN-BEGIN:|106-getter|2|
        return newRecognitionCommand;
    }
    //</editor-fold>//GEN-END:|106-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: cancelCommand1 ">//GEN-BEGIN:|111-getter|0|111-preInit
    /**
     * Returns an initiliazed instance of cancelCommand1 component.
     * @return the initialized component instance
     */
    public Command getCancelCommand1() {
        if (cancelCommand1 == null) {//GEN-END:|111-getter|0|111-preInit
            // write pre-init user code here
            cancelCommand1 = new Command("Cancel", Command.CANCEL, 0);//GEN-LINE:|111-getter|1|111-postInit
            // write post-init user code here
        }//GEN-BEGIN:|111-getter|2|
        return cancelCommand1;
    }
    //</editor-fold>//GEN-END:|111-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: backCommand ">//GEN-BEGIN:|113-getter|0|113-preInit
    /**
     * Returns an initiliazed instance of backCommand component.
     * @return the initialized component instance
     */
    public Command getBackCommand() {
        if (backCommand == null) {//GEN-END:|113-getter|0|113-preInit
            // write pre-init user code here
            backCommand = new Command("Back", Command.BACK, 0);//GEN-LINE:|113-getter|1|113-postInit
            // write post-init user code here
        }//GEN-BEGIN:|113-getter|2|
        return backCommand;
    }
    //</editor-fold>//GEN-END:|113-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: helpCommand ">//GEN-BEGIN:|117-getter|0|117-preInit
    /**
     * Returns an initiliazed instance of helpCommand component.
     * @return the initialized component instance
     */
    public Command getHelpCommand() {
        if (helpCommand == null) {//GEN-END:|117-getter|0|117-preInit
            // write pre-init user code here
            helpCommand = new Command("Help", Command.HELP, 0);//GEN-LINE:|117-getter|1|117-postInit
            // write post-init user code here
        }//GEN-BEGIN:|117-getter|2|
        return helpCommand;
    }
    //</editor-fold>//GEN-END:|117-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: backCommand1 ">//GEN-BEGIN:|121-getter|0|121-preInit
    /**
     * Returns an initiliazed instance of backCommand1 component.
     * @return the initialized component instance
     */
    public Command getBackCommand1() {
        if (backCommand1 == null) {//GEN-END:|121-getter|0|121-preInit
            // write pre-init user code here
            backCommand1 = new Command("Back", Command.BACK, 0);//GEN-LINE:|121-getter|1|121-postInit
            // write post-init user code here
        }//GEN-BEGIN:|121-getter|2|
        return backCommand1;
    }
    //</editor-fold>//GEN-END:|121-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: helpForm ">//GEN-BEGIN:|119-getter|0|119-preInit
    /**
     * Returns an initiliazed instance of helpForm component.
     * @return the initialized component instance
     */
    public Form getHelpForm() {
        if (helpForm == null) {//GEN-END:|119-getter|0|119-preInit
            // write pre-init user code here
            helpForm = new Form("Help", new Item[] { getStringItem(), getStringItem1(), getStringItem2() });//GEN-BEGIN:|119-getter|1|119-postInit
            helpForm.addCommand(getBackCommand1());
            helpForm.setCommandListener(this);//GEN-END:|119-getter|1|119-postInit
            // write post-init user code here
        }//GEN-BEGIN:|119-getter|2|
        return helpForm;
    }
    //</editor-fold>//GEN-END:|119-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: stringItem ">//GEN-BEGIN:|129-getter|0|129-preInit
    /**
     * Returns an initiliazed instance of stringItem component.
     * @return the initialized component instance
     */
    public StringItem getStringItem() {
        if (stringItem == null) {//GEN-END:|129-getter|0|129-preInit
            // write pre-init user code here
            stringItem = new StringItem("iKnowU: ", "This application aims to recognize the persons from a photo using a dedicated server. To do that, you should choose a server address (IP) and a listen port using the Settings button from Menu. These settings should be provided by your administrator. The maximum matched faces refers to the maximum number of candidates for each detected face.", Item.PLAIN);//GEN-LINE:|129-getter|1|129-postInit
            // write post-init user code here
        }//GEN-BEGIN:|129-getter|2|
        return stringItem;
    }
    //</editor-fold>//GEN-END:|129-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: stringItem1 ">//GEN-BEGIN:|130-getter|0|130-preInit
    /**
     * Returns an initiliazed instance of stringItem1 component.
     * @return the initialized component instance
     */
    public StringItem getStringItem1() {
        if (stringItem1 == null) {//GEN-END:|130-getter|0|130-preInit
            // write pre-init user code here
            stringItem1 = new StringItem("Choosing a Photo: ", "The Select Photo button from Menu allows you to select a face image using the FileBrowser.  Before sending the photo to the server (clicking on Recognize button), you will be able to see a preview of the selected photo.");//GEN-LINE:|130-getter|1|130-postInit
            // write post-init user code here
        }//GEN-BEGIN:|130-getter|2|
        return stringItem1;
    }
    //</editor-fold>//GEN-END:|130-getter|2|

    //<editor-fold defaultstate="collapsed" desc=" Generated Getter: stringItem2 ">//GEN-BEGIN:|131-getter|0|131-preInit
    /**
     * Returns an initiliazed instance of stringItem2 component.
     * @return the initialized component instance
     */
    public StringItem getStringItem2() {
        if (stringItem2 == null) {//GEN-END:|131-getter|0|131-preInit
            // write pre-init user code here
            stringItem2 = new StringItem("Interpreting the Results:", "For each detected face you will receive a \"maximum matched number of faces\" which you have selected in the first step. Each candidate will have a sample face photo followed by information extracted from the database (Name, Address, Phone Number, Ocuppation and a recognition confidence percent). Finally, you may choose from these candidates the one you consider to be the best.");//GEN-LINE:|131-getter|1|131-postInit
            // write post-init user code here
        }//GEN-BEGIN:|131-getter|2|
        return stringItem2;
    }
    //</editor-fold>//GEN-END:|131-getter|2|


    /**
     * Returns a display instance.
     * @return the display instance.
     */
    public Display getDisplay () {
        return Display.getDisplay(this);
    }

    /**
     * Exits MIDlet.
     */
    public void exitMIDlet() {
        switchDisplayable (null, null);
        destroyApp(true);
        notifyDestroyed();
    }

    /**
     * Called when MIDlet is started.
     * Checks whether the MIDlet have been already started and initialize/starts or resumes the MIDlet.
     */
    public void startApp() {
        if (midletPaused) {
            resumeMIDlet ();
        } else {
            initialize ();
            startMIDlet ();
        }
        midletPaused = false;
    }

    /**
     * Called when MIDlet is paused.
     */
    public void pauseApp() {
        midletPaused = true;
    }

    /**
     * Called to signal the MIDlet to terminate.
     * @param unconditional if true, then the MIDlet has to be unconditionally terminated and all resources has to be released.
     */
    public void destroyApp(boolean unconditional) {
    }


    /**
     * This methog resizes an image by resampling its pixels
     * @param src The image to be resized
     * @return The resized image
     */
    private Image resizeImage(Image src, int screenWidth, int screenHeight) {

        int srcWidth = src.getWidth();
        int srcHeight = src.getHeight();
        Image tmp = Image.createImage(screenWidth, srcHeight);
        Graphics g = tmp.getGraphics();
        int ratio = (srcWidth << 16) / screenWidth;
        int pos = ratio / 2;

        //Horizontal Resize

        for (int x = 0; x < screenWidth; x++) {
            g.setClip(x, 0, 1, srcHeight);
            g.drawImage(src, x - (pos >> 16), 0, Graphics.LEFT | Graphics.TOP);
            pos += ratio;
        }

        Image resizedImage = Image.createImage(screenWidth, screenHeight);
        g = resizedImage.getGraphics();
        ratio = (srcHeight << 16) / screenHeight;
        pos = ratio / 2;

        //Vertical resize

        for (int y = 0; y < screenHeight; y++) {
            g.setClip(0, y, screenWidth, 1);
            g.drawImage(tmp, 0, y - (pos >> 16), Graphics.LEFT | Graphics.TOP);
            pos += ratio;
        }
        return resizedImage;

    }

public static String[] split(String splitStr, String delimiter) {
     StringBuffer token = new StringBuffer();
     Vector tokens = new Vector();
     // split
     char[] chars = splitStr.toCharArray();
     for (int i=0; i < chars.length; i++) {
         if (delimiter.indexOf(chars[i]) != -1) {
             // we bumbed into a delimiter
             if (token.length() > 0) {
                 tokens.addElement(token.toString());
                 token.setLength(0);
             }
         } else {
             token.append(chars[i]);
         }
     }
     // don't forget the "tail"...
     if (token.length() > 0) {
         tokens.addElement(token.toString());
     }
     // convert the vector into an array
     String[] splitArray = new String[tokens.size()];
     for (int i=0; i < splitArray.length; i++) {
         splitArray[i] = (String)tokens.elementAt(i);
     }
     splitStr = null;
     return splitArray;
 }

    /*
     *  ----------Network communication methods--------------
     */ 

    /**
     * Sends a image over the network
     * @param output
     * @param input
     * @param imageInput
     * @param imageSize
     * @return
     */
    private int sendImage(DataOutputStream output, DataInputStream input,
            InputStream imageInput, int imageSize) {

        try {
            //send image size
            if (DEBUG_ON)
                System.out.println("Sending image size ("+imageSize+") ...");

            transferWaitScreen.setText("Sending image size ("+imageSize+") ...");

            /*ByteArrayOutputStream byteStream = new ByteArrayOutputStream(4);
            DataOutputStream intOutput = new DataOutputStream(byteStream);
            intOutput.writeInt(imageSize);
            intOutput.close();
            */
            //output.write(byteStream.toByteArray(), 0, byteStream.size());
            output.writeInt(imageSize);
            output.flush();
            //byteStream.close();
            
            if (DEBUG_ON)
                System.out.println("Image size ("+imageSize+") sent.");

            if (USE_ACK) {
                receiveACK(input);
                sendACK(output);
            }
            transferWaitScreen.setText("Image size ("+imageSize+") sent.");

            if (DEBUG_ON)
                System.out.println("Sending image data ...");

            transferWaitScreen.setText("Sending image... " + imageSize + " bytes left.");

            byte[] byteBuff = new byte[PACKAGE_MAX_SIZE];

            while (imageSize > PACKAGE_MAX_SIZE) {
                imageInput.read(byteBuff, 0, PACKAGE_MAX_SIZE);
                sendBytes(output, input, byteBuff, PACKAGE_MAX_SIZE);
                imageSize -= PACKAGE_MAX_SIZE;
                transferWaitScreen.setText("Sending image... " + imageSize + " bytes left.");
            }
            imageInput.read(byteBuff, 0, imageSize);
            sendBytes(output, input, byteBuff, imageSize);

            if (DEBUG_ON)
                System.out.println("Image data sent.");

            transferWaitScreen.setText("Image sent.");
            
        } catch (IOException e) {
            Alert a = new Alert("Communication Error",
            "Error sending image to server. Retry!", null, AlertType.ERROR);
            a.setTimeout(Alert.FOREVER);
            a.setCommandListener(this);
            nextDisplayable = selectedImageForm;
            switchDisplayable(a, selectedImageForm);
            return 1;
        }
        
        return 0;
    }

     /**
     * Receives a image from a given input stream
     * @param output
     * @param input
     * @return received image as byte[]
     */
    private byte[] receiveImage(DataOutputStream output, DataInputStream input) {
        ByteArrayOutputStream byteOS = null;
        
        try {
            //receive image size
            if (DEBUG_ON)
                System.out.println("Receiving image size ...");

            transferWaitScreen.setText("Receiving image size ...");

            int imgSize = input.readInt();

            if (DEBUG_ON)
                System.out.println("Image size ("+imgSize+") received.");

            if (USE_ACK) {
                sendACK(output);
                receiveACK(input);
            }

            transferWaitScreen.setText("Image size ("+imgSize+") received.");

            byteOS = new ByteArrayOutputStream(imgSize);
            
            if (DEBUG_ON)
                System.out.println("Receiving image data ...");

            transferWaitScreen.setText("Receiving image... " + imgSize + " bytes left.");

            byte[] byteBuff = new byte[PACKAGE_MAX_SIZE];
            
            while (imgSize > PACKAGE_MAX_SIZE) {
                receiveBytes(output, input, byteBuff, PACKAGE_MAX_SIZE);
                imgSize -= PACKAGE_MAX_SIZE;
                byteOS.write(byteBuff, 0, PACKAGE_MAX_SIZE);
                transferWaitScreen.setText("Receiving image... " + imgSize + " bytes left.");
            }
            receiveBytes(output, input, byteBuff, imgSize);
            byteOS.write(byteBuff, 0, imgSize);


            if (DEBUG_ON)
                System.out.println("Image data received.");

            transferWaitScreen.setText("Image received.");

        } catch (IOException e) {
            Alert a = new Alert("Communication Error",
            "Error receiving image from server. Retry!", null, AlertType.ERROR);
            a.setTimeout(Alert.FOREVER);
            a.setCommandListener(this);
            nextDisplayable = selectedImageForm;
            switchDisplayable(a, selectedImageForm);
            return null;
        }

        return byteOS.toByteArray();
    }


    /**
     * Sends a byte buffer to a given output stream
     * @param output
     * @param byteBuf
     * @param bufLen
     * @return  0 if bytes were sent OK and 1 if not
     * @throws IOException
     */
    private int sendBytes(DataOutputStream output, DataInputStream input,
            byte byteBuf[], int bufLen) throws IOException {

        if (DEBUG_ON)
            System.out.println("Sending " + bufLen + " bytes ...");

	output.write(byteBuf, 0, bufLen);
	output.flush();

 	if (DEBUG_ON)
            System.out.println("Sent " + bufLen + " bytes.");

        if (USE_ACK) {
            receiveACK(input);
            sendACK(output);
        }

        return 0;
    }

    /**
     * Receives a byte buffer from a given input stream
     * @param input
     * @param byteBuff
     * @param bufLen
     * @return the number of bytes received
     * @throws IOException
     */
    private int receiveBytes(DataOutputStream output, DataInputStream input,
            byte byteBuff[], int bufLen) throws IOException {
        int totalBytes = 0;
        int crtRecvBytes = 0;

        if (DEBUG_ON)
            System.out.print("Receiving " + bufLen + " bytes ...");

        while (totalBytes < bufLen) {
            crtRecvBytes = input.read(byteBuff, totalBytes, bufLen - totalBytes);
            totalBytes += crtRecvBytes;
        }

        if (DEBUG_ON)
            System.out.print("Received " + bufLen + " bytes.");
        
        if (USE_ACK) {
            sendACK(output);
            receiveACK(input);
        }

        return totalBytes;
    }

      
    /**
     * Sends ACK after a message using the given outputStream
     * @param output
     */
    private void sendACK(DataOutputStream output) throws IOException{
        if (DEBUG_ON)
            System.out.println("Sending ACK ...");

        int ack = 0;
        output.write(ack);
        output.flush();

        if (DEBUG_ON)
            System.out.println("ACK sent.");
    }

    /**
     * Receives ACK from the given socket
     * @param input
     */
    private void receiveACK(DataInputStream input) throws IOException {
	if (DEBUG_ON)
            System.out.println("Receiving ACK ...");

	int ack = (int) input.read();

	if (DEBUG_ON)
            System.out.println("ACK recieved.");
    }

}
