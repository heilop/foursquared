<?php
// Process Icon, if any
$icon = ''; // Prevent E_STRICT warning
if (array_key_exists(0, $venue->categories)) {
  $icon_src = $venue->categories[0]->icon;
  $icon = '<img src="' . $icon_src . '" />';
}
?>

<li><?php echo $icon ?> <?php echo $venue->name ?></li>