/*
 * 
 *(function($) {
  $(document).ready(function() {
    alert('...');
    $("#edit-templates-template").change(privatemsgTemplatesChangeTemplate);
  });

  function privatemsgTemplatesChangeTemplate() {
    var selected_template = $(this).val();
    var data = {'selected_template': selected_template};

    $.ajax({
      url: Drupal.settings.basePath + 'messages/templates/js_change_template',
      type: "POST",
      data: data,
      dataType: 'json',
      success: function(result) {
        $('#edit-body').val(result.data);
      }
    });
  }
}(jQuery));*/