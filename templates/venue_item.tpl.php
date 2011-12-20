<?php
// Process Icon, if any
$icon = ''; // Prevent E_STRICT warning
if (array_key_exists(0, $venue->categories)) {
  $icon_obj = $venue->categories[0]->icon;
  $icon_src = $icon_obj->prefix . $icon_obj->sizes[0] . $icon_obj->name;
  $icon = '<img src="' . $icon_src . '" />';
}
?>

<li><?php echo $icon ?> <?php echo $venue->name ?></li>