# BSP Dungeon Generation Dump (Given The BSP is generated with seed 23456)
# Data information generated from BSP-Dungeon's TreeParser() function, Formatted with help from ChatGPT


###### Some clarification

The number next to the node represent the hierarchy in the tree. The highest (#) value of the node (ex. Node #) is the root node
and the descending (#) of the Nodes represent the depth level of node. Nodes of the same (#) value are on the same depth level.


# Summary

* **Total Nodes:** 29
* **Leaf Nodes:** 15

---

## High Level Abstraction representation of the tree

# BSP Tree (Clean View)

```
Container (Generated with seed 23456)
├── Left (Node 20)
│   ├── 19
│   │   ├── 18
│   │   │   ├── 17
│   │   │   └── 17
│   │   └── 18
│   │       ├── 17
│   │       └── 17
│   └── 19
│       ├── 18
│       │   ├── 17
│       │   └── 17
│       └── 18
│           ├── 17
│           └── 17
│
└── Right (Node 20)
    ├── 19
    │   ├── 18
    │   │   ├── 18
    │   │   └── 18
    │   └── 19
    │       ├── 18
    │       │   ├── 17
    │       │   └── 17
    │       └── 18
    │           ├── 17
    │           │   ├── 16
    │           │   └── 16
    │           └── 17
    │               ├── 16
    │               └── 16
```

---

## Node Format

Name: String
Position: (x, y)
Size: width x height
Left Child: int indices
Right Child: int indices

---

###### Node Info Below

------------
Name Container
x 0
y 0
width 150
height 100
left_child, [Index]: 1
right_child, [Index]: 16
------------
Name 20
x 0
y 0
width 64
height 100
left_child, [Index]: 2
right_child, [Index]: 9
------------
Name 19
x 0
y 0
width 64
height 50
left_child, [Index]: 3
right_child, [Index]: 6
------------
Name 18
x 0
y 0
width 33
height 50
left_child, [Index]: 4
right_child, [Index]: 5
------------
Name 17
x 0
y 0
width 33
height 21
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        s
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<   m       t            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 0
y 21
width 33
height 29
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<  m                     >
<       $                >
<        $               >
<         $     m        >
<          $             >
<           $            >
<            $           >
<             $          >
<              $         >
<               $        >
<                $       >
<           t            >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 18
x 33
y 0
width 31
height 50
left_child, [Index]: 7
right_child, [Index]: 8
------------
Name 17
x 33
y 0
width 31
height 24
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<    $       $       $   >
<    $       $       $   >
<    $       $       $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<        $       $       >
<   m    $   l   $       >
<        $       $       >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 33
y 24
width 31
height 26
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<    $       $       $   >
<    $       $       $   >
<    $       $       $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<        $       $       >
<   m    $   l   $       >
<        $       $       >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 19
x 0
y 50
width 64
height 50
left_child, [Index]: 10
right_child, [Index]: 13
------------
Name 18
x 0
y 50
width 64
height 30
left_child, [Index]: 11
right_child, [Index]: 12
------------
Name 17
x 0
y 50
width 30
height 30
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 30
y 50
width 34
height 30
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        s
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<   m       t            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 18
x 0
y 80
width 64
height 20
left_child, [Index]: 14
right_child, [Index]: 15
------------
Name 17
x 0
y 80
width 33
height 20
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 33
y 80
width 31
height 20
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 20
x 64
y 0
width 86
height 100
left_child, [Index]: 17
right_child, [Index]: 20
------------
Name 19
x 64
y 0
width 86
height 26
left_child, [Index]: 18
right_child, [Index]: 19
------------
Name 18
x 64
y 0
width 51
height 26
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 18
x 115
y 0
width 35
height 26
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<    $       $       $   >
<    $       $       $   >
<    $       $       $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<        $       $       >
<   m    $   l   $       >
<        $       $       >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 19
x 64
y 26
width 86
height 74
left_child, [Index]: 21
right_child, [Index]: 24
------------
Name 18
x 64
y 26
width 86
height 20
left_child, [Index]: 22
right_child, [Index]: 23
------------
Name 17
x 64
y 26
width 33
height 20
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 97
y 26
width 53
height 20
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 18
x 64
y 46
width 86
height 54
left_child, [Index]: 25
right_child, [Index]: 28
------------
Name 17
x 64
y 46
width 86
height 27
left_child, [Index]: 26
right_child, [Index]: 27
------------
Name 16
x 64
y 46
width 42
height 27
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<    $       $       $   >
<    $       $       $   >
<    $       $       $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<    $   $   $   $   $   >
<        $       $       >
<   m    $   l   $       >
<        $       $       >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 16
x 106
y 46
width 44
height 27
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 17
x 64
y 73
width 86
height 27
left_child, [Index]: 29
right_child, [Index]: 30
------------
Name 16
x 64
y 73
width 46
height 27
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<           m            >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 16
x 110
y 73
width 40
height 27
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<   $$                   >
<                        >
<       $$      $$       >
<       $$      $$       >
<       $$      $$       >
<          tlt           >
<       $$      $$       >
<       $$      $$       >
<                        >
<       $$      $$       >
<                        >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&

# Notes

* Nodes with `left_child = -1` and `right_child = -1` are **leaf nodes (rooms)**
* This BSP is a **valid full binary tree**
* Coordinates are relative to the global grid
* Structure reflects recursive spatial partitioning

---


---
