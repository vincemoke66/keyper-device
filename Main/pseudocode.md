```c
while (true)
    display rfid symbol animation

    read rfid
    if rfid == ""
        continue 

    getStudent(rfid)
    if status code != 404 
        display student name  
        display student school id

        while readingKeyRFIDTime < readingKeyRFIDDuration

            read rfid
            if rfid == ""
                continue 

            getKey(rfid)

            if status code == 404 
                display "Invalid Key Tag"
                delay(2s)
                break inner loop
                
            if key["type"] == "available"
                borrowKey(rfid)
                display "Key borrowed"
                delay(2s)
                break

            if key["type"] == "borrowed"
                returnKey(rfid)
                display "Key returned"
                delay(2s)
                break

    getKey(rfid)
    if status code == 404 
        display "Invalid Tag"
        delay(2s)
        continue

    if key["type"] == "available"
        display "Cannot borrow key, School ID is needed"
        delay(2s)
        continue

    if key["type"] == "borrowed"
        returnKey(rfid)
        display "Key returned"
        delay(2s)
        continue
```