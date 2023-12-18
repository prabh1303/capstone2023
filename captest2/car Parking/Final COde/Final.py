import random
import os
import cv2
from datetime import datetime
import pandas as pd
import serial
import imutils
import easyocr
import numpy as np


def NumberExtraction(image,reader):
    result=None
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    bfilter = cv2.bilateralFilter(gray, 11, 11, 17)
    edged = cv2.Canny(bfilter, 30, 200)
    keypoints = cv2.findContours(edged.copy(), cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    contours = imutils.grab_contours(keypoints)
    contours = sorted(contours, key = cv2.contourArea, reverse = True)[:10]
    location = None
    for contour in contours:
        approx = cv2.approxPolyDP(contour, 10, True)
        if len(approx) == 4:
            location = approx
            break
    if location is not None:
        mask = np.zeros(gray.shape, np.uint8)
        new_image = cv2.drawContours(mask, [location], 0, 255, -1)
        new_image = cv2.bitwise_and(new_image, new_image, mask = mask)
        (x, y) = np.where(mask == 255)
        (x1, y1) = (np.min(x), np.min(y))
        (x2, y2) = (np.max(x), np.max(y))
        cropped_image = gray[x1:x2+3, y1:y2+3]
        reader = easyocr.Reader(['en'])
        temp = reader.readtext(cropped_image)
        if any(temp):
            result=temp[0][1]
    return result


def filePresent():
    fileList = os.listdir()
    if 'Entries.xlsx' not in fileList :
        data=[]
        df = pd.DataFrame(data,columns=['Slot', 'Vechicle Number', 'Entry Date', 'Entry Time', 'Exit Date', 'Exit Time'])
        df.to_excel("Entries.xlsx",index=False)


def readExcel():
    main_out = pd.read_excel('Entries.xlsx')
    return main_out

#Function to check whether the Entry point is Exited or not
def isComplete(main_out,serial_Number):
    Flag=0
    for i in range(len(main_out)):
        str = f"E{serial_Number}"
        Info=main_out.loc[i,['Slot', 'Vechicle Number', 'Entry Date', 'Entry Time', 'Exit Date', 'Exit Time']]
        Info = list(Info)
        if(Info[0]==str):
            Flag=1

    if Flag==1:
        Info = main_out.loc[i, ['Slot', 'Vechicle Number', 'Entry Date', 'Entry Time', 'Exit Date', 'Exit Time']]
        Info = list(Info)
        if(Info[5] == "Null"):
            return False
        else:
            return True
    else:
        return True


def isEntryPresent(main_out,serial_Number):
    main_out=readExcel()
    flag = False
    for i in range(len(main_out)):
        str = f"E{serial_Number}"
        Info=main_out.loc[i,['Slot', 'Vechicle Number', 'Entry Date', 'Entry Time', 'Exit Date', 'Exit Time']]
        Info = list(Info)
        print(f"Info[5] is {Info}")
        if(Info[0]==str):
            flag = True
        else:
            pass

    return flag


def runFunction(main_out,slots,vechicle_number):
    main_out=readExcel()
    if ('E' in slots):
        main_out = readExcel()
        if (len(main_out) == 0):

            # Taking current Date and Time
            now = datetime.now()
            curr_time = now.strftime("%H:%M:%S")
            curr_date = now.strftime("%d/%m/%y")

            # #Extracting Slot Number from Slot

            # Adding data to output dictionary
            output = {"Slot": [slots],
                      "Vechicle Number": [vechicle_number],
                      "Entry Date": [curr_date],
                      "Entry Time": [curr_time],
                      "Exit Date": ["Null"],
                      "Exit Time": ["Null"]}

            df = pd.DataFrame(output)
            main_out = pd.concat([main_out,df])

            print(output)
            print(main_out)
            main_out.to_excel("Entries.xlsx", index=False)
            main_out=readExcel()

        else:
            main_out = readExcel()

            # Taking Current Date and Time
            now = datetime.now()
            curr_time = now.strftime("%H:%M:%S")
            curr_date = now.strftime("%d/%m/%y")

            # Extracting Slot Number from Slot
            serial_Number = slots[1]

            # Checking whether Slot is exited or not
            complete = isComplete(main_out,serial_Number)
            if (isComplete(main_out, serial_Number) ==False):
                print("Waiting for Exit can not add")

            else:
                main_out = readExcel()

                #Creating a dictionary of current working data
                output = {"Slot": [slots],
                          "Vechicle Number": [vechicle_number],
                          "Entry Date": [curr_date],
                          "Entry Time": [curr_time],
                          "Exit Date": ["Null"],
                          "Exit Time": ["Null"]}


                #Concatinating the main_out data frame and output data frame
                df = pd.DataFrame(output)
                main_out = pd.concat([main_out,df])

                print(output)
                print(main_out)
                main_out.to_excel("Entries.xlsx", index=False)
                main_out = readExcel()


    elif('X' in slots):
        main_out = readExcel()
        serial_Number = slots[1]

        #Checking whether Excel Sheet is Empty or not
        if (len(main_out) == 0):
            print("Nothing to Exit")

        elif (isEntryPresent(main_out,serial_Number)==False):
            print("Entry is not Present")

        else:
            main_out = readExcel()
            serial_Number = slots[1]

            # Checking whether the Entry is already exit or not
            present = isEntryPresent(main_out,serial_Number)
            if(not present):
               print("Element not present")

            else:
                main_out = readExcel()
                now = datetime.now()
                curr_time = now.strftime("%H:%M:%S")
                curr_date = now.strftime("%d/%m/%y")
                str = f"E{serial_Number}"


                # Update the exit date for the specified slot
                main_out.loc[main_out['Slot'] == str, 'Exit Date'] = curr_date
                main_out.loc[main_out['Slot'] == str, 'Exit Time'] = curr_time



                print(main_out)
                main_out.to_excel("Entries.xlsx", index=False)
                main_out = readExcel()



    elif ('q' in slots):
        exit()

    else:
        print("Invalid")


    #Appending the final dataframe into Excel
    main_out.to_excel("Entries.xlsx", index=False)
    main_out = readExcel()


if __name__ == '__main__':

    # Serial port configuration
    ser = serial.Serial("COM3", 9600)  # Replace "COM1" with your serial port
    ser.flush()
    reader = easyocr.Reader(['en'])
    filePresent()
    main_out = pd.read_excel('Entries.xlsx')
    print(main_out)
    print(len(main_out))
    print(type(main_out))
    vid = cv2.VideoCapture(1)

    while (True):

        ret, image = vid.read()
        if not ret:
            print("Failed to capture frame")
            break

        if ser.inWaiting()>0:
            inp=str(ser.readline())
            inp=inp[2:4]
            print(inp)
            if "E" in inp:
                result=NumberExtraction(image,reader)
                print(result)
                while result==None:
                    print("Invalid Case")
                    ret, image = vid.read()
                    result=NumberExtraction(image,reader)
                    print(result)
                    cv2.imshow('Preview',image)
                    if cv2.waitKey(1) & 0xFF == ord('q'):
                        ch = input("Thanks for using our app \n"
                        "do you want to view the records\n"
                        "1. Press 's' to show \n"
                        "2. Press 'q' to exit")
                        if ch == 's':
                            os.startfile('Entries.xlsx')
                            break
                        elif key == ord('q'):
                            break
                        break   
                else:
                    runFunction(main_out,inp,result)
            elif "X" in inp:
                runFunction(main_out,inp,"")
            
        cv2.imshow('Preview', image)

        key = cv2.waitKey(1)

        if key == ord('q'):
            ch = input("Thanks for using our app \n"
                  "do you want to view the records\n"
                  "1. Press 's' to show \n"
                  "2. Press 'q' to exit")
            if ch == 's':
                os.startfile('Entries.xlsx')
                break
            elif key == ord('q'):
                break
            break

        
    vid.release()
    cv2.destroyAllWindows()
