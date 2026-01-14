from pikepdf import Pdf,Name

A4_LANDSCAPE = (842, 595)  # points

def merge_then_2up(main_pdf, insert_pdf, output_pdf):
    with (
        Pdf.open(main_pdf) as main,
        Pdf.open(insert_pdf) as template,
        Pdf.new() as merged
    ):
        #out.Root.PageLayout = Name.TwoPageLeft
        for main_page in reversed(main.pages):
            
            # Add ALL pages from insert PDF
            #for ins_page in template.pages:
            merged.pages.insert(0,template.pages[0])  # Assuming we only want the first page of the template
            
            # Add one page from main PDF
            merged.pages.insert(0,main_page)

            
        for main_page in main.pages:
            # 1. create one blank A4 landscape page
            sheet = merged.add_blank_page(page_size=A4_LANDSCAPE)

            # 2. copy pages INTO output PDF
            left = merged.copy_page(main_page)
            right = merged.copy_page(template_page)

            # 3. overlay them side by side
            sheet.add_overlay(
                left,
                rect=(0, 0, A4_LANDSCAPE[0] / 2, A4_LANDSCAPE[1])
            )

            sheet.add_overlay(
                right,
                rect=(A4_LANDSCAPE[0] / 2, 0, A4_LANDSCAPE[0], A4_LANDSCAPE[1])
            )

        out.save(output_pdf)
        
    


if __name__ == "__main__":
    merge_then_2up(
        "b1_skript.pdf",
        "Template.pdf",
        "output.pdf"
    )


# if __name__ == "__main__":
#     side_by_side_pdf(
#     main_pdf = "b1_skript.pdf",
#     insert_pdf = "Template.pdf",
#     output_pdf = "output.pdf"
#     )
