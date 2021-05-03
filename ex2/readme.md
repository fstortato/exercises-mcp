## Task
N workers are sitting at a round table and are assembling computers. 
Each worker can access the computer parts lying in front of him, two gloves placed to the left and right, and a wrench placed in the middle of the table.
To build a computer, each worker executes a series of steps:
1. Connect the parts
2. Wear the right glove
3. Wear the left glove
4. Pick up the wrench
5. Tighten the bolts on the casing
6. Put back the equipments

The array of working gloves l glove and the wrench l wrench should be implemented by global PThread lock variables. 
Picking up and putting back the gloves and the wrench should be implemented by locking and unlocking the lock variables.
Actions 1 and 5 can be implemented by printing a useful message.
