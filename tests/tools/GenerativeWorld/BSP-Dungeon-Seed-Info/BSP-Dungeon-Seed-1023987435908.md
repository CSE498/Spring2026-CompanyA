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
right_child, [Index]: 8
------------
Name 20
x 0
y 0
width 37
height 100
left_child, [Index]: 2
right_child, [Index]: 7
------------
Name 19
x 0
y 0
width 37
height 63
left_child, [Index]: 3
right_child, [Index]: 4
------------
Name 18
x 0
y 0
width 37
height 22
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
y 22
width 37
height 41
left_child, [Index]: 5
right_child, [Index]: 6
------------
Name 17
x 0
y 22
width 37
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
y 42
width 37
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
Name 19
x 0
y 63
width 37
height 37
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
Name 20
x 37
y 0
width 113
height 100
left_child, [Index]: 9
right_child, [Index]: 24
------------
Name 19
x 37
y 0
width 82
height 100
left_child, [Index]: 10
right_child, [Index]: 17
------------
Name 18
x 37
y 0
width 31
height 100
left_child, [Index]: 11
right_child, [Index]: 14
------------
Name 17
x 37
y 0
width 31
height 58
left_child, [Index]: 12
right_child, [Index]: 13
------------
Name 16
x 37
y 0
width 31
height 23
left_child, [Index]: -1
right_child, [Index]: -1
^^^^^^^^^^^^^^^^^^^^^^^^^^
<                        >
<    $$$$$$$$$$$$$$$$$   >
<    $     lllll     $   >
<    $               $   >
<    $$$$$$$   $$$$$$$   >
<                        >
&&&&&&&&&&&&&&&&&&&&&&&&&&
------------
Name 16
x 37
y 23
width 31
height 35
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
x 37
y 58
width 31
height 42
left_child, [Index]: 15
right_child, [Index]: 16
------------
Name 16
x 37
y 58
width 31
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
Name 16
x 37
y 80
width 31
height 20
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
x 68
y 0
width 51
height 100
left_child, [Index]: 18
right_child, [Index]: 23
------------
Name 17
x 68
y 0
width 51
height 70
left_child, [Index]: 19
right_child, [Index]: 20
------------
Name 16
x 68
y 0
width 51
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
x 68
y 20
width 51
height 50
left_child, [Index]: 21
right_child, [Index]: 22
------------
Name 15
x 68
y 20
width 51
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
Name 15
x 68
y 40
width 51
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
Name 17
x 68
y 70
width 51
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
x 119
y 0
width 31
height 100
left_child, [Index]: 25
right_child, [Index]: 28
------------
Name 18
x 119
y 0
width 31
height 41
left_child, [Index]: 26
right_child, [Index]: 27
------------
Name 17
x 119
y 0
width 31
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
Name 17
x 119
y 20
width 31
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
Name 18
x 119
y 41
width 31
height 59
left_child, [Index]: 29
right_child, [Index]: 30
------------
Name 17
x 119
y 41
width 31
height 22
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
x 119
y 63
width 31
height 37
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