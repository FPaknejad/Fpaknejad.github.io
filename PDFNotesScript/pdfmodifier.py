from pikepdf import Pdf,Name

A4_LANDSCAPE = (842, 595)  # points

def merge_then_2up(main_pdf, insert_pdf, output_pdf): 
    with ( 
          Pdf.open(main_pdf) as main, 
          Pdf.open(insert_pdf) as template, 
          Pdf.new() as out 
          ): 
        for main_page in reversed(main.pages): 
            out.pages.insert(0,template.pages[0]) 
            
            # Add one page from main PDF 
            out.pages.insert(0,main_page)
            out.save(output_pdf)
        
    


if __name__ == "__main__":
    merge_then_2up(
        "b1_skript.pdf",
        "temp_d.pdf",
        "b1_skript_notesd.pdf"
    )
