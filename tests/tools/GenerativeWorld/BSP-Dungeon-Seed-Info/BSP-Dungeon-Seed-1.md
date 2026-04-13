# BSP Dungeon Generation Dump (Given The BSP is generated with seed 1)
# Data information generated from BSP-Dungeon's TreeParser() function, Formatted with help from ChatGPT


###### Some clarification #######
The number next to the node represent the hierarchy in the tree. The highest (#) value of the node (ex. Node #) is the root node
and the descending (#) of the Nodes represent the depth level of node. Nodes of the same (#) value are on the same depth level.

## High Level Abstraction representation of the tree 
Container 
# BSP Tree (Clean View)

# Total Leaf + Node Count in BSP Tree
# Leaf: 15
# Tree: 29

------------
Name Container
x 0
y 0
width 150
height 100
left_child, [Index]: 1
right_child, [Index]: 6
------------
Name 20
x 0
y 0
width 150
height 29
left_child, [Index]: 2
right_child, [Index]: 3
------------
Name 19
x 0
y 0
width 55
height 29
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
Name 19
x 55
y 0
width 95
height 29
left_child, [Index]: 4
right_child, [Index]: 5
------------
Name 18
x 55
y 0
width 36
height 29
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
Name 18
x 91
y 0
width 59
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
Name 20
x 0
y 29
width 150
height 71
left_child, [Index]: 7
right_child, [Index]: 22
------------
Name 19
x 0
y 29
width 107
height 71
left_child, [Index]: 8
right_child, [Index]: 13
------------
Name 18
x 0
y 29
width 107
height 21
left_child, [Index]: 9
right_child, [Index]: 12
------------
Name 17
x 0
y 29
width 62
height 21
left_child, [Index]: 10
right_child, [Index]: 11
------------
Name 16
x 0
y 29
width 32
height 21
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
Name 16
x 32
y 29
width 30
height 21
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
Name 17
x 62
y 29
width 45
height 21
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
Name 18
x 0
y 50
width 107
height 50
left_child, [Index]: 14
right_child, [Index]: 17
------------
Name 17
x 0
y 50
width 107
height 20
left_child, [Index]: 15
right_child, [Index]: 16
------------
Name 16
x 0
y 50
width 52
height 20
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
Name 16
x 52
y 50
width 55
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
x 0
y 70
width 107
height 30
left_child, [Index]: 18
right_child, [Index]: 19
------------
Name 16
x 0
y 70
width 39
height 30
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
x 39
y 70
width 68
height 30
left_child, [Index]: 20
right_child, [Index]: 21
------------
Name 15
x 39
y 70
width 32
height 30
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
Name 15
x 71
y 70
width 36
height 30
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
x 107
y 29
width 43
height 71
left_child, [Index]: 23
right_child, [Index]: 26
------------
Name 18
x 107
y 29
width 43
height 47
left_child, [Index]: 24
right_child, [Index]: 25
------------
Name 17
x 107
y 29
width 43
height 22
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
x 107
y 51
width 43
height 25
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
x 107
y 76
width 43
height 24
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